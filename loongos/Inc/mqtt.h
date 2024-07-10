/*-------------------------------------------------*/
/*            超子说物联网STM32系列开发板          */
/*-------------------------------------------------*/
/*                                                 */
/*            实现MQTT协议功能的头文件             */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __MQTT_H
#define __MQTT_H


#define  TOPIC_NUM    3                              	 //需要订阅的最大Topic数量
#define  TOPIC_SIZE   64                             	 //存放Topic字符串名称缓冲区长度
#define  CLIENT_TXBUFF_SIZE  1024		//客户端发送缓存区大小

typedef struct{
	char ClientID[64];                         //存放客户端ID的缓冲区
	char Username[64];                         //存放用户名的缓冲区
	char Passward[64];                         //存放密码的缓冲区
	char Pack_buff[256];              //发送报文的缓冲区
	int  MessageID;                            //记录报文标识符
	int  Fixed_len;                       	   //固定报头长度
	int  Variable_len;                         //可变报头长度
	int  Payload_len;                          //有效负荷长度
	char Stopic_Buff[TOPIC_NUM][TOPIC_SIZE];   //包含的是订阅的主题列表
	char cmdbuff[400];                         //保存推送的数据中的命令数据部分
}MQTT_CB;  
#define MQTT_CB_LEN         sizeof(MQTT_CB)    //结构体长度 

typedef struct{       
	char flag;           //定义一个变量, 0：表示当次接收还未完成 1：表示接收完成 
	int datalen;              //定义一个变量，保存当次串口接收到的数据的大小  
    unsigned char buff[CLIENT_TXBUFF_SIZE];		//客户端发送缓存区
}CLIENT_CB;                                        
#define CLIENT_CB_LEN         sizeof(CLIENT_CB)    //结构体长度 

extern MQTT_CB   Aliyun_mqtt;  //外部变量声明，用于连接阿里云mqtt的结构体
extern CLIENT_CB clientTx;     //创建一个数据发送结构体

void IoT_Parameter_Init(char productKey[], char deviceName[], char deviceSecret[], int secretLen);  //函数声明，云服务器初始化参数，得到客户端ID，用户名和密码
void MQTT_ConectPack(void);                                                       //函数声明，MQTT CONNECT报文       建立连接
void MQTT_DISCONNECT(void);                                                       //函数声明，MQTT DISCONNECT报文    断开连接
void MQTT_Subscribe(char topicbuff[TOPIC_NUM][TOPIC_SIZE],int, char);    //函数声明，MQTT SUBSCRIBE报文     订阅Topic
void MQTT_UNSubscribe(char *);                                                    //函数声明，MQTT UNSUBSCRIBE报文   取消订阅Topic
void MQTT_PingREQ(void);                                                          //函数声明，MQTT PING报文          保活心跳包
void MQTT_PublishQs0(char *,char *,int);                                          //函数声明，MQTT PUBLISH报文 等级0 发布数据
void MQTT_PublishQs1(char *,char *,int);                                          //函数声明，MQTT PUBLISH报文 等级1 发布数据
void MQTT_DealPushdata_Qs0(char *,int);                                  //函数声明，处理服务器发来的等级0的推送数据
void TxDataBuf_Deal(char *, int);                                        //函数声明，向发送缓冲区添加数据
int powdata(int , int);
#endif
