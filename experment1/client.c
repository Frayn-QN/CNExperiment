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

#define MAXLINE 120

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

void client_func(int connfd) {
    char buf[MAXLINE] = {0};
    while(1) {
        if(fgets(buf, MAXLINE, stdin) == NULL) {
            perror("fgets error");
            break;
        }

        // 结束标记 
        if(strcmp(buf, "EXIT\n") == 0) {
            break;
        }
    }
}

int main(int argc, char** argv) {
    if(argc != 3) {
        printf("Usage: %s <ip_address> <port>\n", argv[0]);
        return 1;
    }

    // 定义部分
    char* server_ip = argv[1];
    char* server_port = argv[2];

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
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    if(connfd == -1) {
        perror("socket error");
        return 1;
    }

    // 请求连接
    if(connect(connfd, (struct sockaddr*)&server_addr, server_addrlen) == -1) {
        perror("connect error");
        return 1;
    }

    printf("[cli] server[%s:%s] is connected!\n", server_ip, server_port);

    // 受理业务
    client_func(connfd);
    
    // 关闭连接
    close(connfd);
    return 0;
}