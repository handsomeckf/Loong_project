#ifndef __CLIENT_H
#define __CLIENT_H
#include "mqtt.h"

extern int clientfd;	//连接服务器套接字描述符

#define PRODUCTKEY_LEN         11           //三元组 ProductKey     字符串长度        
#define DEVICENAME_LEN         32           //三元组 DeviceName     字符串长度        
#define DEVICESECRET_LEN       32           //三元组 DeviceSecret   字符串长度
#define VERSION_LEN            23           //程序固件版本号字符串长度

/*---------------------------------------------------------------*/
/*--------------保存阿里云证书信息的结构体---------------*/
/*---------------------------------------------------------------*/
typedef struct{
	char  ProductKeyBuff[PRODUCTKEY_LEN+1];                           //保存ProductKey的缓冲区
	char  DeviceNameBuff[DEVICENAME_LEN+1];                           //保存DeviceName的缓冲区
	char  DeviceSecretBuff[DEVICESECRET_LEN+1];                       //保存DeviceSecret的缓冲区
	char  Version_ABuff[VERSION_LEN+1];                               //保存程序固件版本的缓冲区
}AliyunInfo_CB;   
#define ALIINFO_STRUCT_LEN        sizeof(AliyunInfo_CB)                         //EEPROM内保存阿里云证书信息的结构体 长度 

/*---------------------------------------------------------------*/
/*-------------------用于各种系统参数的结构体--------------------*/
/*---------------------------------------------------------------*/
typedef struct{
	unsigned int SystemTimer;                //用于全局统计的计时变量  
	unsigned int PingTimer;                  //用于记录发送PING数据包的计时器 
	unsigned int PingTimeouter;              //用于记录PING检查事件的计时器
    unsigned int DataTimer;                   //用于记录RGB灯时间
	unsigned int SysEventFlag;               //发生各种事件的标志变量	
}Sys_CB;
#define SYS_STRUCT_LEN         sizeof(Sys_CB)                                   //用于各种系统参数的 Sys_CB结构体 长度 
	
/*---------------------------------------------------------------*/
/*-----------------------系统事件发生标志定义--------------------*/
/*---------------------------------------------------------------*/
#define CONNECT_EVENT         0x80000000          						        //表示连接上服务器事件发生
#define PING_SENT             0x40000000          						        //表示PING保活包发送事件发生
#define PING_CHECK            0x20000000          						        //表示PING检查事件发生


/*---------------------------------------------------------------*/
/*-------------------------  分  割  线  ------------------------*/
/*--------------  模板程序中 分割线以上部分 不修改 --------------*/
/*-------  模板程序中 分割线以下部分 根据不同的功能修改 ---------*/
/*---------------------------------------------------------------*/


/*---------------------------------------------------------------*/
/*----------------------自定义事件发生标志定义-------------------*/
/*---------------------------------------------------------------*/
//根据功能自行添加，注意不要和系统事件发生标志定义的数值一样
//需要多少，自行添加
//#define GET_DATA_EVENT         0x00000001          //表示该上传数据事件发生


/*---------------------------------------------------------------*/
/*-----------------------功能属性标识符宏定义--------------------*/
/*---------------------------------------------------------------*/
//所有程序中需要用到的功能属性的标识符在此定义
//标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define NODE1TEMP            "Node1Temp"    //功能属性节点1温度标识符
#define NODE1HUM             "Node1Hum"     //功能属性节点1湿度标识符
#define NODE1ILLUM           "Node1Illum"   //功能属性节点1光照强度标识符
#define NODE1ELE             "Node1Ele"     //功能属性节点1电量标识符

#define NODE2TEMP            "Node2Temp"    //功能属性节点2温度标识符
#define NODE2HUM             "Node2Hum"     //功能属性节点2湿度标识符
#define NODE2ILLUM           "Node2Illum"   //功能属性节点2光照强度标识符
#define NODE2ELE             "Node2Ele"     //功能属性节点2电量标识符

#define NODE3TEMP            "Node3Temp"    //功能属性节点3温度标识符
#define NODE3HUM             "Node3Hum"     //功能属性节点3湿度标识符
#define NODE3ILLUM           "Node3Illum"   //功能属性节点3光照强度标识符
#define NODE3ELE             "Node3Ele"     //功能属性节点3电量标识符

//... ... 需要多少，自行添加

//---------------------事件报警标识符宏定义--------------------//
//所有程序中需要用到的事件报警的标识符在此定义
//标识符名称必须和服务器后台设置的完全一样，大小写也必须一样
#define EVENT1                "事件1标识符"     //事件1标识符
#define EVENT2                "事件2标识符"     //事件2标识符
//... ... 需要多少，自行添加

//------------各种外部变量声明，便于其他源文件调用变量-----------//
extern Sys_CB  SysCB;                //外部变量声明，用于各种系统参数的结构体
extern CLIENT_CB clientRx;       	//创建一个数据接受结构体
extern CLIENT_CB clientTx;       	//创建一个数据接受结构体
extern AliyunInfo_CB AliInfoCB;  //文本内保存的阿里云证书信息结构体

void client_init(void);	//异步I/O初始化函数
void passive_event(unsigned char *data, int datalen);	//被动事件
void WiFi_PropertyVersion(void);	//向服务器上传版本号
void WiFi_PropertyPost(char * postdata);	//向服务器上传数据

#endif 
