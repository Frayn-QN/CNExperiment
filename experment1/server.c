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

int sigint_flag = 0;
void handle_sigint(int sig) {
    printf("[srv] SIGINT is coming!\n");
    sigint_flag = 1;
}

int main(int argc, char** argv) {
    if(argc != 3) {
        printf("Usage: %s <ip_address> <port> <vcd>\n", argv[0]);
        return 1;
    }

    // 定义部分

    // 安装SIGINT信号处理器
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // 设置socket
    
    


}