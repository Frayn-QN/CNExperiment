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

#define MAXLINE 1024

int sigint_flag = 0;
void handle_sigint(int sig) {
    printf("[srv] SIGINT is coming!\n");
    sigint_flag = 1;
}

ssize_t rio_readn(int fd, void *usrbuf, size_t n)//无缓冲输入
{
	size_t nleft = n;
	ssize_t nread;
	char *bufp = (char*)usrbuf;
	
	while(nleft > 0)
	{
		if((nread = read(fd, bufp, nleft)) < 0)
		{
			if(errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else if(nread == 0)
			break;/*EOF*/
		
		nleft -= nread;
		bufp += nread;
	}
	
	return (n - nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)//无缓冲输出
{
	size_t nleft = n;
	ssize_t nwritten;
	char *bufp = (char*)usrbuf;
	
	while(nleft > 0)
	{
		if((nwritten = write(fd, bufp, nleft)) <= 0)
		{
			if(errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}
		
		nleft -= nwritten;
		bufp += nwritten;
	}
	
	return n;
}


void server_func(int connfd, const char* vcd) {
    while(1) {
        char buf[MAXLINE] = {0};

        // 接收消息
        int valread = rio_readn(connfd, buf, MAXLINE);
        if(valread <= 0) {
            if (valread == -1){
                perror("rio_readn error");
            }
            break;
        }
        printf("[ECH_RQT]%s\n", buf);

        // 发送回声
        sprintf(buf, "(%s)%s\0", vcd, buf);
        if(rio_writen(connfd, buf, strlen(buf)) == -1) {
            perror("rio_writen error");
            break;
        }
    }
}

int main(int argc, char** argv) {
    if(argc != 4) {
        printf("Usage: %s <ip_address> <port> <vcd>\n", argv[0]);
        return 1;
    }

    // 定义部分
    char* server_ip = argv[1];
    char* server_port = argv[2];
    char* server_vcd = argv[3];

    // 安装SIGINT信号处理器
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // 设置服务器地址
    struct sockaddr_in server_addr;  
    socklen_t server_addrlen = sizeof(server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(server_port));
    if(inet_pton(AF_IENT, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton error");
        return 1;
    }
    

    // 创建socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1) {
        perror("socket error");
        return 1;
    }

    // 绑定socket
    if(bind(listenfd, (struct sockaddr *)&server_addr, server_addrlen) == -1) {
        perror("bind error");
        return 1;
    }

    // 监听
    if(listen(listenfd, BACKLOG) == -1) {
        perror("listen error");
        return 1;
    }
    printf("[srv] server[%s:%s][%s] [%s] is initializing!\n", server_ip, server_port, server_vcd);

    // 受理业务
    while(!sigint_flag) {
        int connfd = accept(listenfd, (struct sockaddr *)&server_addr, &server_addrlen);
        if(connfd == -1) {
            perror("accept error");
            return 1;
        }

        // 获取客户端信息
        struct sockaddr_in client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        if(getpeername(new_socket, (struct sockaddr *)&client_addr, &client_addrlen) == -1) {
            perror("getpeername error");
            return 1;
        }
        char client_ip[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL) {
            perror("inet_ntop error");
            return 1;
        }

        printf("[srv] client[%s:%d] is accepted!\n", client_ip, ntohs(client_address.sin_port));
        server_func(connfd, server_vcd);
        printf("[srv] client[%s:%d] is closed!\n", client_ip, ntohs(client_address.sin_port));
        close(connfd);
    }
    
    close(listenfd);
    printf("[srv] listenfd is closed!\n");
    printf("[srv] server is to return!\n");

    return 0;
}
