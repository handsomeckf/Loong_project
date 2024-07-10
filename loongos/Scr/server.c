/***************************************************************
 文件名 : server.c
 作者 : xiyuxing
 版本 : V1.0
 描述 : 配置网络服务器
 其他 : 无
 日志 : 初版 V1.0 2022/8/30 xiyuxing创建
 ***************************************************************/
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "server.h"

#define MAGIC "1234567890"
#define MAGIC_LEN 11
#define IP_BUFFER_SIZE 65536
#define RECV_TIMEOUT_USEC 100000
/*
__attrubte__ ((packed)) 的作用就是告诉编译器取消结构在编译过程中的优化对齐,按照实际占用字节数进行对齐
*/
//定义ICMP回环结构体 
struct __attribute__((__packed__)) icmp_echo {
    // header
    uint8_t type; //类型 
    uint8_t code; //代码 
    uint16_t checksum;//校验和 

    uint16_t ident;//标识符 
    uint16_t seq; //符号 

    // data
    double sending_ts;//发送时间 
    char magic[MAGIC_LEN]; //字符串 
};


int serverfd;
char shutdownFlag;
ClientConn clientConn[3];
int ci;


/**
 ** 获取时间戳
 **/
static double get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + ((double)tv.tv_usec) / 1000000;
}

/**
 ** 计算校验和
 **/
static uint16_t calculate_checksum(unsigned char* buffer, int bytes)
{
    uint32_t checksum = 0;
    unsigned char* end = buffer + bytes;

    // odd bytes add last byte and reset end
    if (bytes % 2 == 1) {
        end = buffer + bytes - 1;
        checksum += (*end) << 8;
    }

    // add words of two bytes, one by one
    while (buffer < end) {
        checksum += (buffer[0] << 8) + buffer[1];

        // add carry if any
        uint32_t carray = checksum >> 16;
        if (carray != 0) {
            checksum = (checksum & 0xffff) + carray;
        }

        buffer += 2;
    }
    // negate it
    checksum = ~checksum;

    return checksum & 0xffff;
}

/**
 ** 发送回显请求
 **/
static int send_echo_request(int sock, struct sockaddr_in* addr, int ident, int seq) {
    // allocate memory for icmp packet
    struct icmp_echo icmp;
    bzero(&icmp, sizeof(icmp));

    // fill header files
    icmp.type = 8;
    icmp.code = 0;
    icmp.ident = htons(ident);
    icmp.seq = htons(seq);

    // fill magic string
    strncpy(icmp.magic, MAGIC, MAGIC_LEN);

    // fill sending timestamp
    icmp.sending_ts = get_timestamp();

    // calculate and fill checksum
    icmp.checksum = htons(
        calculate_checksum((unsigned char*)&icmp, sizeof(icmp))
    );

    // send it
    /*
		sendto() 用来将数据由指定的 socket 传给对方主机
    	参数1：socket文件描述符
		参数2：发送数据的首地址
		参数3：数据长度
		参数4：默认方式发送
		参数5：存放目的主机的IP和端口信息
		参数6: 参数5的长度 
	*/
    int bytes = sendto(sock, &icmp, sizeof(icmp), 0,(struct sockaddr*)addr, sizeof(*addr));
    if (bytes == -1) {
        return -1;
    }
    return 0;
}

/**
 ** 实现recv_echo_reply用于接收ICMP回显应答报文
 **/
static int recv_echo_reply(int sock, int ident, struct sockaddr_in* addr) {
    // allocate buffer
    //定义缓冲区 
    unsigned char buffer[IP_BUFFER_SIZE];
    //sockaddr_in 结构体
    struct sockaddr_in peer_addr;

    // receive another packet
    socklen_t addr_len = sizeof(peer_addr);
    /*
		recvfrom()本函数用于从（已连接）套接口上接收数据，并捕获数据发送源的地址 
		s：标识一个已连接套接口的描述字。 
		buf：接收数据缓冲区。 
		len：缓冲区长度。 
		flags：调用操作方式。 
		from：（可选）指针，指向装有源地址的缓冲区。 
		fromlen：（可选）指针，指向from缓冲区长度值。 
	*/
    int bytes = recvfrom(sock, buffer, sizeof(buffer), 0,
        (struct sockaddr*)&peer_addr, &addr_len);
    if (bytes == -1) {
        perror("recv error");
        return -1;
    }

    // 比较是否是同一个ip回传的包
    if( peer_addr.sin_addr.s_addr != addr->sin_addr.s_addr) {
        perror("IP error");
        return -1;
    }

	//IP头部长度 
    int ip_header_len = (buffer[0] & 0xf) << 2;
    //判断接收到的字节是否足够转化成struct icmp_echo结构体
    if (bytes < ip_header_len+sizeof(struct icmp_echo)) {
        perror("length error");
        return -1;
    }
    // find icmp packet in ip packet
    //从 IP 报文中取出 ICMP 报文
	struct icmp_echo* icmp = (struct icmp_echo*)(buffer + ip_header_len);

    // check type
    if (icmp->type != 0 || icmp->code != 0) {
        perror("type error");
        return -1;
    }

    // match identifier
    //ntohs()是一个函数名，作用是将一个16位数由网络字节顺序转换为主机字节顺序
    if (ntohs(icmp->ident) != ident) {
        perror("ident error");
        return -1;
    }

    // print info
    /*
    printf("ping %s seq=%-5d %8.2fms\n",
        inet_ntoa(peer_addr.sin_addr),
        ntohs(icmp->seq),
        (get_timestamp() - icmp->sending_ts) * 1000
    );
    */
    return 0;
}

/**
 ** 发送PING包
 **/
void ping(ClientConn *clientConnPing)
{
    int flag = 0; // 进入ping默认连接断开
    clientConnPing->client_addr.sin_port = 0;
    // create raw socket for icmp protocol
    //创建一个原始套接字，协议类型为 IPPROTO_ICMP 
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (sock == -1) {
        perror("create raw socket");
        goto clear;
    }

    // set socket timeout option
    struct timeval tv = {0};
    tv.tv_sec = 1;
    int ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (ret == -1) {
        perror("set socket option");
        goto clear;
    }

    int ident = getpid();//取得进程识别码

    for (int seq=1; seq<=3 && clientConnPing->connectFlag; seq++) {
        // send it
        ret = send_echo_request(sock, &clientConnPing->client_addr, ident, seq);
        if (ret == -1) {
            perror("Send failed");
            continue;
        }

        // try to receive and print reply
        ret = recv_echo_reply(sock, ident, &clientConnPing->client_addr);
        if (ret == -1) {
            perror("Receive failed");
        }
        else if(ret == 0){
            flag = 1; //表示ping通过，连接存在
        }
        sleep(1);
    }
    // 判断是否在ping的过程中断开
    if(clientConnPing->connectFlag) clientConnPing->connectFlag = flag;
    if(clientConnPing->connectFlag){    //ping通了
        printf("ping %s connection\n", inet_ntoa(clientConnPing->client_addr.sin_addr) );
    }
    //printf("connectFlag: %d \n", clientConnPing->connectFlag);
clear:
    close(sock);

}




/**
 ** 服务器初始化
 **/
void server_init(void)
{
    struct sockaddr_in server_addr = {0};

    /* 打开套接字，得到套接字描述符 */
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > serverfd) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }
    shutdownFlag = 1;
    /* 将套接字与指定端口号进行绑定 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    if (0 > bind(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        perror("bind error");
        close(serverfd);
        exit(EXIT_FAILURE);
    }


    /* 关闭套接字 */
    //close(serverfd);
}

/**
 ** 信号处理函数，当串口有数据可读时，会跳转到该函数执行
 **/
static void io_handler(int sig, siginfo_t *info, void *context)
{
    int ret;


    if((sig == clientConn[0].sig + SIGRTMIN) && clientConn[0].connectFlag) ret = 0;
    else if((sig == clientConn[1].sig + SIGRTMIN) && clientConn[1].connectFlag) ret = 1;
    else if((sig == clientConn[2].sig + SIGRTMIN) && clientConn[0].connectFlag) ret = 2;
    else return;

    /* 判断串口是否有数据可读 */
    if (POLL_IN == info->si_code) {
        // 读数据
        clientConn[ret].datalen = recv(clientConn[ret].connfd, clientConn[ret].recvBuf, RECVBUFNUM, 0);
        clientConn[ret].getDataFlag = 1;
    }
}

/**
 ** 异步I/O初始化函数
 **/
static void server_io_init()
{
    struct sigaction sigatn;
    int flag;

    /* 使能异步I/O */
    flag = fcntl(clientConn[ci].connfd, F_GETFL);
    flag |= O_ASYNC;
    fcntl(clientConn[ci].connfd, F_SETFL, flag);

    /* 设置异步I/O的所有者 */
    fcntl(clientConn[ci].connfd, F_SETOWN, getpid());

    /* 指定实时信号SIGRTMIN作为异步I/O通知信号 */
    fcntl(clientConn[ci].connfd, F_SETSIG, SIGRTMIN+clientConn[ci].sig);

    /* 为实时信号SIGRTMIN注册信号处理函数 */
    sigatn.sa_sigaction = io_handler;   //当串口有数据可读时，会跳转到io_handler函数
    sigatn.sa_flags = SA_SIGINFO;
    sigemptyset(&sigatn.sa_mask);
    sigaction(SIGRTMIN+clientConn[ci].sig, &sigatn, NULL);
}

/**
 ** 阻塞等待客户端连接
 **/
void wait_connect(void)
{
    int connfd;
    struct sockaddr_in client_addr;

    char ip_str[20] = {0};
    int addrlen = sizeof(client_addr);
    memset(&client_addr, 0, addrlen);  //清空客户端的ip和端口号
    /* 使服务器进入监听状态 */
    if (0 > listen(serverfd, 50)) {
        perror("listen error");
        close(serverfd);
        exit(EXIT_FAILURE);
    }

    /* 阻塞等待客户端连接 */
    connfd = accept(serverfd, (struct sockaddr *)&client_addr, &addrlen);
    if (0 > connfd) {
        perror("accept error");
        close(serverfd);
        exit(EXIT_FAILURE);
    }


    for(ci=0; ci<3; ci++){
        if(!clientConn[ci].connectFlag) break;
    }
    if(ci>=3){close(connfd); return;}

    clientConn[ci].connfd = connfd;
    clientConn[ci].client_addr = client_addr;
    clientConn[ci].connectFlag = 1; //标志位置1
    clientConn[ci].sig = ci+1;

    printf("There is a client access...\n");
    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str, sizeof(ip_str));
    printf("IP address of the client host: %s\n", ip_str);
    printf("Port number of the client process: %d\n", client_addr.sin_port);
    server_io_init();
}



/**
 ** 将串口接收信号从信号掩码中添加或删除
 **/
void sig_procmask(int sig)
{
    /* 定义信号集 */
    sigset_t sig_set;

    /* 将信号集初始化为空 */
    sigemptyset(&sig_set);
    /* 向信号集中添加 SIGINT 信号 */
    sigaddset(&sig_set, SIGRTMIN+((sig+1)%4));
    sigaddset(&sig_set, SIGRTMIN+((sig+2)%4));
    sigaddset(&sig_set, SIGRTMIN+((sig+3)%4));
    
    /* 向进程的信号掩码中添加或删除信号 */
    if(-1 == sigprocmask(SIG_BLOCK, &sig_set, NULL)) {
        perror("sigprocmask error");
        exit(-1);
    }

}
