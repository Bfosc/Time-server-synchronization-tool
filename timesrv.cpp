#include <iostream>
#include <cstring>
#include <ctime>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 1024
#define LISTENQ 1024

void *thread_func(void *arg) {
    int connfd = *(int *)arg;
    char buff[MAXLINE];
    time_t ticks;

    ticks = time(NULL);
    snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
    write(connfd, buff, strlen(buff));
    close(connfd);

    return nullptr;
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    pthread_t tid;

    // 创建监听套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket error");
        return 1;
    }

    // 初始化服务器地址结构
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr =(INADDR_ANY);
    servaddr.sin_port = htons(8319);

    // 绑定套接字
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind error");
        close(listenfd);
        return 1;
    }

    // 开始监听
    if (listen(listenfd, LISTENQ) < 0) {
        perror("listen error");
        close(listenfd);
        return 1;
    }

    for (;;) {
        // 接受连接
        connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
        if (connfd < 0) {
            perror("accept error");
            continue;
        }

        // 创建线程处理连接
        if (pthread_create(&tid, NULL, thread_func, (void *)&connfd) != 0) {
            perror("pthread_create error");
            close(connfd);
        }
        pthread_detach(tid); // 确保线程结束后资源能被回收
    }

    close(listenfd);
    return 0;
}
