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
#include <sys/wait.h>

#define MAXLINE 1024

int sigint_flag = 0;
void handle_sigint(int sig) {
    printf("[srv](%d) SIGINT is coming!\n", getpid());
    sigint_flag = 1;
}

void handle_sigchld(int sig) {
    pid_t child_pid;
    pid_t main_pid = getpid();
    int stat;
    while((child_pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("[srv](%d)[chd](%d) Child has terminated!\n", main_pid, child_pid);
    }
}

int sig_num = 0;
void handle_sigpipe(int sig) {
    sig_num = sig;
    pid_t main_pid = getpid();
    printf("[srv](%d) SIGPIPE is coming!\n", main_pid);
}

void server_func(int connfd, int vcd, pid_t pid) {
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


int main(int argc, char** argv) {
    if(argc != 4) {
        printf("Usage: %s <ip_address> <port> <veri_code>\n", argv[0]);
        return 1;
    }

    // 定义部分
    char* server_ip = argv[1];
    char* server_port = argv[2];
    char* server_vcd = argv[3];
    pid_t main_pid = getpid();
    pid_t child_pid = 0;

    // 信号处理
    int res = -1;
    // 安装SIGINT信号处理器
    struct sigaction sa_int, old_sa_int;
    sa_int.sa_flags = 0;
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    res = sigaction(SIGINT, &sa_int, old_sa_int);
    if(res) {
        return -1;
    }

    // 安装SIGCHLD信号处理器
    struct sigaction sa_chd, old_sa_chd;
    sa_chd.sa_flags = 0;
    sa_chd.sa_handler = handle_sigchld;
    sigemptyset(&sa_chd.sa_mask);
    res = sigaction(SIGCHLD, &sa_chd, &old_sa_chd);
    if(res) {
        return -2;
    }

    // 安装SIGPIPE信号处理器
    struct sigaction sa_pipe, old_sa_pipe;
    sa_pipe.sa_flags = 0;
    sa_pipe.sa_flags |= SA_RESTART;
    sa_pipe.sa_handler = handle_sigpipe;
    sigemptyset(&sa,sa_mask);
    res = sigaction(SIGPIPE, &sa_pipe, &old_sa_pipe);
    if(res) {
        return -3;
    }

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
    if(listen(listenfd, 3) == -1) {
        perror("listen error");
        return 1;
    }
    printf("[srv](%d)[srv_sa](%s:%s)[vcd](%s) Server has initialized!\n", server_ip, server_port, server_vcd);

    // 受理业务
    while(!sigint_flag) {
        int connfd = accept(listenfd, (struct sockaddr *)&server_addr, &server_addrlen);
        if(connfd == -1) {
            if(errno == EINTR) {
                continue;
            }
            perror("accept error");
            return 1;
        }

        // 获取客户端信息
        struct sockaddr_in client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        if(getpeername(connfd, (struct sockaddr *)&client_addr, &client_addrlen) == -1) {
            perror("getpeername error");
            return 1;
        }
        char client_ip[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL) {
            perror("inet_ntop error");
            return 1;
        }
        int client_port = ntohs(client_addr.sin_port);
        printf("[srv](%d)[cli_sa](%s:%d) Client is accepted!\n", main_pid, client_ip, client_port);

        // 创建子进程处理客户端请求
        child_pid = fork();
        if(child_pid == -1) {
            perror("fork error");
            return 1;
        }
        else if(child_pid == 0) {// 子进程
            printf("[chd](%d)[ppid](%d) Client process is created!\n", child_pid, main_pid);
            close(listenfd);

            server_func(connfd, atoi(server_vcd), child_pid);

            printf("[chd](%d)[ppid](%d)[cli_sa](%s:%d) Client is closed!\n", child_pid, main_pid, client_ip, client_port);
            close(connfd);
            printf("[chd](%d)[ppid](%d) connfd is closed!\n", child_pid, main_pid);
            printf("[chd](%d)[ppid](%d) Client process is to return!\n", child_pid, main_pid);
            return 0;
        }
        
        close(connfd);
    }
    
    close(listenfd);
    printf("[srv](%d) listenfd is closed!\n", main_pid);
    printf("[srv](%d) server is to return!\n", main_pid);

    return 0;
}