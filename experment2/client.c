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

#define MAXLINE 80

void client_func(int connfd, int cid, pid_t pid) {
    while(1) {
       char buf[MAXLINE] = {0};

        if(fgets(buf, MAXLINE, stdin) == NULL) {
            perror("fgets error");
            break;
        }

        // 结束标记 
        if(strcmp(buf, "EXIT\n") == 0) {
            break;
        }

        printf("[cli](%d)[cid](%d)[ECH_RQT] %s", pid, cid, buf);
        
        // 发送编号
        short cid_h = (short)cid;
        short cid_n = htons(cid_h);
        if(write(connfd, &cid_n, 2) == -1) {
            perror("write error");
            break;
        }

        // 发送消息
        if(write(connfd, buf, strlen(buf)) == -1) {
            perror("write error");
            break;
        }
        
        // 接收验证码
        short vcd_n = 0;
        if(read(connfd, &vcd_n, 2) == -1) {
            perror("read error");
            break;
        }
        short vcd_h = ntohs(vcd_n);

        // 接收回声
        memset(buf, 0, MAXLINE);
        if(read(connfd, buf, MAXLINE) == -1) {
            perror("read error");
        }
        printf("[cli](%d)[vcd](%d)[ECH_REP] %s", pid, vcd_h, buf);
     
    }
}

int main(int argc, char** argv) {
    if(argc != 4) {
        printf("Usage: %s <ip_address> <port> <cid>\n", argv[0]);
        return 1;
    }

    // 定义部分
    char* server_ip = argv[1];
    char* server_port = argv[2];
    char* cid = argv[3];
    pid_t pid = getpid();

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

    printf("[cli](%d)[srv_sa](%s:%s) Server is connected!\n", pid, server_ip, server_port);

    // 受理业务
    client_func(connfd, atoi(cid), pid);
    
    // 关闭连接
    close(connfd);
    printf("[cli](%d) connfd is closed!\n", pid);
    printf("[cli](%d) Client is to return!\n", pid);
    return 0;
}