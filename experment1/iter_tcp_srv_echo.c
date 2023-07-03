#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAXLINE 1024
char* server_ip = NULL;
char* server_port = NULL;
char* server_vcd = NULL;

typedef struct node
{
    int data;
    struct node* lchild;
    struct node* rchild;
}node;

void print(int data) {
    if(data == 9) {
        start();
    }
}

node *CreatTree()
{
	int i;
	node *pNode[11] = {0};
 
	for ( i = 0; i < 10; i++)
	{
		pNode[i] = (node*) malloc(sizeof(node));
 
		if (!pNode[i])
		{
			printf("malloc error!\n");
			exit(EXIT_FAILURE);
		}
 
		pNode[i]->lchild = NULL;
		pNode[i]->rchild = NULL;
		pNode[i]->data = i+1;
	}
 
	for ( i = 0; i < 10/2; i++)
	{
		pNode[i]->lchild = pNode[ 2 * (i + 1) - 1];
 
		pNode[i]->rchild = pNode[ 2 * (i + 1) + 1 - 1];
	}
 
	return pNode[0];
}

//中序遍历
int MidOrderTraverse(node *root, void (*visit)(int) )
{
	if ( NULL == root)
	{
		return 1;
	}
 
	if ( MidOrderTraverse( root->lchild, visit) )
	{
		(*visit)(root->data);
 
		if ( MidOrderTraverse( root->rchild, visit) )
		{
			return 1;
		}
	}
 
	return 0;
}

int sigint_flag = 0;
void handle_sigint(int sig) {
    printf("[srv] SIGINT is coming!\n");
    sigint_flag = 1;
}

void server_func(int connfd, const char* vcd) {
    while(1) {
        char buf[MAXLINE] = {0};

        // 接收消息
        int valread = read(connfd, buf, MAXLINE);
        if(valread <= 0) {
            if (valread == -1){
                perror("read error");
            }
            break;
        }
        printf("[ECH_RQT]%s", buf);

        // 发送回声
        char rep[MAXLINE] = {0};
        sprintf(rep, "(%s)%s", vcd, buf);
        if(write(connfd, rep, strlen(rep)) == -1) {
            perror("write error");
            break;
        }
    }
}

void start() {    
    // 设置服务器地址
    struct sockaddr_in server_addr;
    socklen_t server_addrlen = sizeof(server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(server_port));
    if(inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }
    

    // 创建socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    // 绑定socket
    if(bind(listenfd, (struct sockaddr *)&server_addr, server_addrlen) == -1) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    // 监听
    if(listen(listenfd, 3) == -1) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }
    printf("[srv] server[%s:%s][%s] is initializing!\n", server_ip, server_port, server_vcd);

    // 受理业务
    while(!sigint_flag) {
        int connfd = accept(listenfd, (struct sockaddr *)&server_addr, &server_addrlen);
        if(connfd == -1) {
            if(errno == EINTR) {
                continue;
            }
            perror("accept error");
            exit(EXIT_FAILURE);
        }

        // 获取客户端信息
        struct sockaddr_in client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        if(getpeername(connfd, (struct sockaddr *)&client_addr, &client_addrlen) == -1) {
            perror("getpeername error");
            exit(EXIT_FAILURE);
        }
        char client_ip[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL) {
            perror("inet_ntop error");
            exit(EXIT_FAILURE);
        }

        printf("[srv] client[%s:%d] is accepted!\n", client_ip, ntohs(client_addr.sin_port));
        server_func(connfd, server_vcd);
        printf("[srv] client[%s:%d] is closed!\n", client_ip, ntohs(client_addr.sin_port));
        close(connfd);
    }
    
    close(listenfd);
    printf("[srv] listenfd is closed!\n");
    printf("[srv] server is to return!\n");
}

int main(int argc, char** argv) {
    if(argc != 4) {
        printf("Usage: %s <ip_address> <port> <veri_code>\n", argv[0]);
        return 1;
    }

    // 定义部分
    server_ip = argv[1];
    server_port = argv[2];
    server_vcd = argv[3];

    // 安装SIGINT信号处理器
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    node* root = CreatTree();
    MidOrderTraverse(root, print);

    return 0;
}