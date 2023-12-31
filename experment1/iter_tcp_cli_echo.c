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

#define MAXLINE 140

char* server_ip = NULL;
char* server_port = NULL;

typedef struct node
{
    int data;
    struct node* lchild;
    struct node* rchild;
}node;

void print(int data) {
    if(data == 7) {
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


//先序遍历
int  PreOrderTraverse(node *root, void (*visit)(int) )
{
	if ( NULL == root)
	{
		return 1;
	}
 
	(*visit)(root->data);
 
	if ( PreOrderTraverse(root->lchild, visit) )
	{
		if ( PreOrderTraverse(root->rchild, visit) )
		{
			return 1;
		}
	}
 
	return 0;
 
}


void client_func(int connfd) {
    while(1) {
        char buf[MAXLINE] = {0};

        if(fgets(buf, MAXLINE, stdin) == NULL) {
            perror("fgets error");
            exit(EXIT_FAILURE);
        }
        printf("[ECH_RQT]%s", buf);

        // 结束标记 
        if(strcmp(buf, "EXIT\n") == 0) {
            break;
        }
        
        // 发送消息
        if(write(connfd, buf, strlen(buf)) == -1) {
            perror("write error");
            exit(EXIT_FAILURE);
        }
        memset(buf, 0, MAXLINE);

        // 接收回声
        if(read(connfd, buf, MAXLINE) == -1) {
            perror("read error");
            exit(EXIT_FAILURE);
        }
        printf("[ECH_REP]%s", buf);
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
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    if(connfd == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    // 请求连接
    if(connect(connfd, (struct sockaddr*)&server_addr, server_addrlen) == -1) {
        perror("connect error");
        exit(EXIT_FAILURE);
    }

    printf("[cli] server[%s:%s] is connected!\n", server_ip, server_port);

    // 受理业务
    client_func(connfd);
    
    // 关闭连接
    close(connfd);
    printf("[cli] connfd is closed!\n");
    printf("[cli] client is to return!\n");
}

int main(int argc, char** argv) {
    if(argc != 3) {
        printf("Usage: %s <ip_address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // 定义部分
    server_ip = argv[1];
    server_port = argv[2];

    node* root = CreatTree();
    PreOrderTraverse(root, print);

    return 0;
}