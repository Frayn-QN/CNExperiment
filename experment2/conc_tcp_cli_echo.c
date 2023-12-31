#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 80
#define MAX_NODE 5

char* server_ip = NULL;
char* server_port = NULL;
char* cid = NULL;
pid_t pid = 0;

int visited[MAX_NODE] = {0};

void print(int node) {
    if(node == 3) {
        start();
    }
}

void DFS(int node, int graph[MAX_NODE][MAX_NODE]) {
    visited[node] = 1;
    print(node);
    for(int i = 0; i < MAX_NODE; i++) {
        if(graph[node][i] == 1 && visited[i] == 0) {
            DFS(i, graph);
        }
    }
}

void client_func(int connfd, int cid, pid_t pid) {
    while(1) {
        char buf[MAXLINE] = {0};

        // 读取命令行
        if(fgets(buf, MAXLINE, stdin) == NULL) {
            perror("fgets error");
            exit(EXIT_FAILURE);
        }
        
        if(buf[59] != '\0' && buf[59] != '\n') {
            buf[60] = '\n';
            buf[61] = '\0';
        }
        printf("[cli](%d)[cid](%d)[ECH_RQT] %s", pid, cid, buf);

        // 结束标记 
        if(strcmp(buf, "EXIT\n") == 0) {
            break;
        }
        
        // 发送PDU
        short cid_h = (short)cid;
        short cid_n = htons(cid_h);
        char rep[MAXLINE] = {0};
        memcpy(rep, &cid_n, 2);
        memcpy(rep+2, buf, strlen(buf));
        if(write(connfd, rep, strlen(rep)) == -1) {
            perror("write error");
            exit(EXIT_FAILURE);
        }
        
        // 接收验证码
        short vcd_n = 0;
        if(read(connfd, &vcd_n, 2) == -1) {
            perror("read error");
            exit(EXIT_FAILURE);
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

void start() {
    // 创建socket
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    if(connfd == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;  
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(server_port));
    server_addr.sin_addr.s_addr = inet_addr(server_ip);


    // 请求连接
    if(connect(connfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect error");
        exit(EXIT_FAILURE);
    }

    printf("[cli](%d)[srv_sa](%s:%s) Server is connected!\n", pid, server_ip, server_port);

    // 受理业务
    client_func(connfd, atoi(cid), pid);
    
    // 关闭连接
    close(connfd);
    printf("[cli](%d) connfd is closed!\n", pid);
    printf("[cli](%d) Client is to return!\n", pid);
}

int main(int argc, char** argv) {
    if(argc != 4) {
        printf("Usage: %s <ip_address> <port> <cid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 定义部分
    server_ip = argv[1];
    server_port = argv[2];
    cid = argv[3];
    pid = getpid();

    int graph[MAX_NODE][MAX_NODE] = {
        {0, 1, 1, 0, 0},
        {1, 0, 1, 0, 0},
        {1, 1, 0, 1, 0},
        {0, 0, 1, 0, 1},
        {0, 0, 0, 1, 0}
    };
    DFS(0, graph);

    return 0;
}