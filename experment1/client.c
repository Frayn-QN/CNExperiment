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

void client_func(int connfd) {
    while(1) {
        char buf[MAXLINE] = {0};

        if(fgets(buf, MAXLINE, stdin) == NULL) {
            perror("fgets error");
            break;
        }
        printf("[ECH_RQT]%s", buf);

        // 结束标记 
        if(strcmp(buf, "EXIT\n") == 0) {
            break;
        }
        
        // 发送消息
        if(write(connfd, buf, strlen(buf)) == -1) {
            perror("write error");
            break;
        }
        memset(buf, 0, MAXLINE);

        // 接收回声
        if(read(connfd, buf, MAXLINE) == -1) {
            perror("read error");
        }
        printf("[ECH_REP]%s", buf);
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
    if(inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
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
    printf("[cli] connfd is closed!\n");
    printf("[cli] client is to return!\n");
    return 0;
}