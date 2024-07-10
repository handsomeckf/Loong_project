#ifndef __SERVER_H
#define __SERVER_H
#include <netinet/in.h>
#define SERVER_PORT     8888    //端口号不能发生冲突,不常用的端口号通常大于5000
#define RECVBUFNUM      512

typedef struct{
    int connfd;
    int sig; //信号掩码
    struct sockaddr_in client_addr;
    char connectFlag;  //客户端连接标志

    char getDataFlag;  //模块数据获取标志
    char recvBuf[RECVBUFNUM];     //网络数据接收缓冲区
    int datalen;
}ClientConn;

extern ClientConn clientConn[3];
extern char shutdownFlag;
extern int ci;  //判断当前的是哪一个ClientConn
void ping(ClientConn *clientConnPing);
void server_init(void);
void wait_connect(void);

void sig_procmask(int __how);

#endif
