/***************************************************************
 文件名 : client.c
 作者 : xiyuxing
 版本 : V1.0
 描述 : 连接云服务器
 其他 : 无
 日志 : 初版 V1.0 2022/9/4 xiyuxing创建
 ***************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <netdb.h>  //域名解析

#include "client.h"
#include "mqtt.h"

Sys_CB  SysCB = {0};                //用于各种系统参数的结构体
AliyunInfo_CB AliInfoCB;  //保存的阿里云证书信息结构体
CLIENT_CB clientRx = {0};       	//创建一个数据接受结构体

int clientfd;	//连接服务器套接字描述符



/**
 ** 读取阿里云三元组信息
 **/
static void triple_read_info()
{
    char triple[128];
    FILE *fp = fopen("triple.txt", "r");
    fread(triple, sizeof(triple), 1, fp);

    sscanf(triple, "/%[^/]/%[^/]/%[^/]/%[^/]/", AliInfoCB.ProductKeyBuff, 
                                                AliInfoCB.DeviceNameBuff, 
                                                AliInfoCB.DeviceSecretBuff,
                                                AliInfoCB.Version_ABuff);

	fclose(fp);
}


/**
 ** 信号处理函数，当网口有数据可读时，会跳转到该函数执行
 **/
static void io_handler(int sig, siginfo_t *info, void *context)
{

    if(SIGRTMIN != sig)
        return;

    /* 判断串口是否有数据可读 */
    if (POLL_IN == info->si_code) {
        // 读数据
        clientRx.datalen = recv(clientfd, clientRx.buff, sizeof(clientRx.buff), 0);
		clientRx.flag = 1;
    }

}

/**
 ** 异步I/O初始化函数
 **/
static void client_io_init(void)
{
    struct sigaction sigatn;
    int flag;

    /* 使能异步I/O */
    flag = fcntl(clientfd, F_GETFL);
    flag |= O_ASYNC;
    fcntl(clientfd, F_SETFL, flag);

    /* 设置异步I/O的所有者 */
    fcntl(clientfd, F_SETOWN, getpid());

    /* 指定实时信号SIGRTMIN作为异步I/O通知信号 */
    fcntl(clientfd, F_SETSIG, SIGRTMIN);	//注意：这里与server.c文件共用了SIGRTMIN+1，如需同时使用client.c、server.c需将其修改为其他信号

    /* 为实时信号SIGRTMIN注册信号处理函数 */
    sigatn.sa_sigaction = io_handler;   //当串口有数据可读时，会跳转到io_handler函数
    sigatn.sa_flags = SA_SIGINFO;
    sigemptyset(&sigatn.sa_mask);
    sigaction(SIGRTMIN, &sigatn, NULL);
}
/**
 ** 客户端初始化并连接阿里云服务器
 **/
void client_init(void)
{
    struct sockaddr_in client_addr = {0};
    struct hostent *h;
    char domainName[128];
    unsigned int ServerIP;                     //存放服务器IP
	int  ServerPort = 1883;                    //存放服务器的端口号

    triple_read_info();

    sprintf(domainName, "%s.iot-as-mqtt.cn-shanghai.aliyuncs.com", AliInfoCB.ProductKeyBuff);
    h = gethostbyname(domainName);
	if(h==NULL)
	{
		printf("Domain name resolution failed!\n");
        exit(EXIT_FAILURE);
	}

    memcpy(&ServerIP,h->h_addr,4);  //拷贝ip地址

    /* 打开套接字，得到套接字描述符 */
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > clientfd) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    /* 调用connect连接远端服务器 */
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(ServerPort);  //端口号
    client_addr.sin_addr.s_addr = ServerIP;//IP地址

    if (0 > connect(clientfd, (struct sockaddr *)&client_addr, sizeof(client_addr))){
        perror("connect error");
        close(clientfd);
        exit(EXIT_FAILURE);
    }

    printf("Connecting to the server successfully...\n\n");
    client_io_init();	//客户端接受异步I/O初始化函数

    IoT_Parameter_Init(AliInfoCB.ProductKeyBuff, AliInfoCB.DeviceNameBuff, AliInfoCB.DeviceSecretBuff, DEVICESECRET_LEN);
	MQTT_ConectPack();                           	   //构建Conect报文，加入发送缓冲区，准备发送
}
/**
 ** 被动事件
 ** @param 1 - data: 数据 
 ** @param 2 - datalen: 数据长度
 **/
void passive_event(unsigned char *data, int datalen)
{	
	int i;                        //用于for循环
	char *StrP;     									 //用于查找特定字符串的位置
	char tempdatabuff[256];  //用于临时构建数据的缓冲区

		
	/*----------------------------------------------*/
	/*                   被动事件                  */
	/*                通用数据处理                  */
	/*         基本每个程序都需要处理的数据         */
	/*----------------------------------------------*/
	
	/*----------------------------------------------*/
	/*               处理CONNACK报文                */
	/*----------------------------------------------*/	
	if((data[0]==0x20)){	                     //如果接收数据的第1个字节是0x20 且 当前是连接上服务器的，进入if
        printf("A CONNACK packet was received...\r\n");                                          //串口输出信息
		switch(data[3]){	 		                                                 //接收数据的第4个字节，表示CONNECT报文是否成功
            case 0x00 : printf("CONNECT packet succeeds!\r\n");                            //串口输出信息
						SysCB.SysEventFlag |= CONNECT_EVENT;         	            //表示MQTT CONNECT报文成功事件发生
                        MQTT_Subscribe(Aliyun_mqtt.Stopic_Buff,TOPIC_NUM,1);         //构建Subscribe报文，加入发送缓冲区，准备发送
						break;                                                       //跳出分支case 0x00                                              
            case 0x01 : printf("Unsupported protocol version!\r\n");     //串口输出信息
						break;                                                       //跳出分支case 0x01   
            case 0x02 : printf("Invalid client identifier!\r\n"); //串口输出信息
						break;                                                       //跳出分支case 0x02 
            case 0x03 : printf("The server is unavailable!\r\n");         //串口输出信息
						break;                                                       //跳出分支case 0x03
            case 0x04 : printf("Invalid user name or password!\r\n");   //串口输出信息
						break;                                                       //跳出分支case 0x04
            case 0x05 : printf("Unauthorized!\r\n");               //串口输出信息
						break;                                                       //跳出分支case 0x05 		
            default   : printf("Unknown state!\r\n");             //串口输出信息
						break;                                                       //跳出分支case default 								
		}		
	}
	/*----------------------------------------------*/
	/*                处理SUBACK报文                */
	/*----------------------------------------------*/
	if((data[0]==0x90)&&(SysCB.SysEventFlag&CONNECT_EVENT)){	                     //如果接收数据的第1个字节是0x90 且 当前是连接上服务器的，进入if
        printf("A SUBACK packet was received...\r\n");                                           //串口输出信息
		for(i=0;i<datalen-4;i++){                                                    //循环查询订阅结果
			switch(data[4+i]){		                                                 //从第5个字节，是订阅结果数据，每个topic有一个结果字节			
				case 0x00 :
                case 0x01 : printf("The %d Topic was successfully subscribed!\r\n",i+1);                //串口输出信息
							break;                                                   //跳出分支                                             
                default   : printf("Failed to subscribe the %d Topic!\r\n",i+1);          //串口输出信息
							break;                                                   //跳出分支 								
			}
		}
        printf("Example Synchronize the current firmware version...\r\n");                 //串口输出信息
		WiFi_PropertyVersion();		                      //同步当前固件版本号	
	}
	/*----------------------------------------------*/
	/*              处理PINGRESP报文                */
	/*----------------------------------------------*/
    if((data[0]==0xD0)&&(SysCB.SysEventFlag&CONNECT_EVENT)){	                     //如果接收数据的第1个字节是0xD0 且 当前是连接上服务器的，进入if
        printf("A PINGRESP packet was received...\r\n");                                         //串口输出信息
		SysCB.SysEventFlag &=~ PING_SENT;                                            //清除PING保活包发送事件发生标志
		if(SysCB.SysEventFlag&PING_CHECK){                                           //判断事件是否发生  
			SysCB.SysEventFlag &=~PING_CHECK;                                        //清除事件标志
			SysCB.PingTimeouter = 0;                                                 //PING检查事件计数器清零
		}			
	}
	/*----------------------------------------------*/
	/*               处理PUBACK报文                  */
	/*----------------------------------------------*/
	if((data[0]==0x40)&&(SysCB.SysEventFlag&CONNECT_EVENT)){	                     //如果接收数据的第1个字节是0x40 且 当前是连接上服务器的，进入if
        printf("A PUBACK packet was received...\r\n");                                           //串口输出信息
        printf("The packet with level 1 identifier %d is successfully sent!\r\n",data[2]*256+data[3]);         //串口输出信息
	}
	/*----------------------------------------------*/
	/*           处理服务器推送来的等级0报文            */
	/*----------------------------------------------*/
	if((data[0]==0x30)&&(SysCB.SysEventFlag&CONNECT_EVENT)){	                     //如果接收数据的第1个字节是0x30 且 当前是连接上服务器的，进入if
        printf("Description The level 0 packet pushed by the server was received...\r\n");                                //串口输出信息
        MQTT_DealPushdata_Qs0(data,datalen);	                                     //处理等级0的推送数据，提取命令数据
        printf("%s\r\n",Aliyun_mqtt.cmdbuff);                                     //串口1输出数据	
	}
}
/**
 ** 向服务器上传版本号
 **/
void WiFi_PropertyVersion(void)
{
    char topicdatabuff[64];        //用于构建发送topic的缓冲区
    char tempdatabuff[64];         //用于临时构建数据的缓冲区

    memset(topicdatabuff,0,64);                                                          //清空临时缓冲区
    sprintf(topicdatabuff,"/ota/device/inform/%s/%s",AliInfoCB.ProductKeyBuff,AliInfoCB.DeviceNameBuff);             //构建发送topic
    memset(tempdatabuff,0,64);                                                           //清空临时缓冲区
    sprintf(tempdatabuff,"{\"id\": 1,\"params\":{\"version\":\"%s\"}}",AliInfoCB.Version_ABuff);   //构建数据
    MQTT_PublishQs1(topicdatabuff,tempdatabuff,strlen(tempdatabuff));                    //等级1的PUBLISH报文，加入发送缓冲区
}
/**
 ** 向服务器上传数据
 ** @param 1 - postdata ：上传的数据
 **/
void WiFi_PropertyPost(char * postdata)
{
    char topicdatabuff[64];       //用于构建发送topic的缓冲区

    memset(topicdatabuff,0,64);                                                          //清空临时缓冲区
    sprintf(topicdatabuff,"/sys/%s/%s/thing/event/property/post",AliInfoCB.ProductKeyBuff,AliInfoCB.DeviceNameBuff); //构建发送topic
    MQTT_PublishQs0(topicdatabuff,postdata,strlen(postdata));                            //等级1的PUBLISH报文，加入发送缓冲区
}

