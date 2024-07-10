#include "multi_thread.h"

MainWindow *w = nullptr;   //全局窗口的全局变量，用于各个线程的通信

/**
 ** 主动事件
 **/
static void ActiveEvent()
{
    char tempdatabuff[256];  //用于临时构建数据的缓冲区
    /*---------------------------------------------*/
    /*               自定义主动事件                  */
    /*---------------------------------------------*/

    for(int i=0; i<3; i++){
        if(w->node[i]->mqttDataFlag){

            // 上传温度
            //printf("Node%dTemp:%0.2f\n",i+1, w->node[i]->tData);
            memset(tempdatabuff,0,256);     //清空临时缓冲区
            sprintf(tempdatabuff,"{\"params\":{\"Node%dTemp\":%0.2f}}",i+1, w->node[i]->tData); //构建数据
            WiFi_PropertyPost(tempdatabuff);            //向服务器上传数据

            // 上传湿度
            //printf("Node%dHum:%d\n",i+1, w->node[i]->hData);
            memset(tempdatabuff,0,256);     //清空临时缓冲区
            sprintf(tempdatabuff,"{\"params\":{\"Node%dHum\":%d}}",i+1, w->node[i]->hData); //构建数据
            WiFi_PropertyPost(tempdatabuff);            //向服务器上传数据

            // 上传光照强度
            //printf("Node%dIllum:%d\n",i+1, w->node[i]->iData);
            memset(tempdatabuff,0,256);     //清空临时缓冲区
            sprintf(tempdatabuff,"{\"params\":{\"Node%dIllum\":%d}}",i+1, w->node[i]->iData); //构建数据
            WiFi_PropertyPost(tempdatabuff);            //向服务器上传数据

            // 上传电池电量
            //printf("Node%dEle:%d\n",i+1, w->node[i]->eData);
            memset(tempdatabuff,0,256);     //清空临时缓冲区
            sprintf(tempdatabuff,"{\"params\":{\"Node%dEle\":%d}}",i+1, w->node[i]->eData); //构建数据
            WiFi_PropertyPost(tempdatabuff);            //向服务器上传数据

            w->node[i]->mqttDataFlag = 0;
        }
    }
}


static void mqtt_connection()
{
    client_init();  //初始化网络服务器
    SysCB.PingTimer = time(NULL);   //避免一开始就发送ping包
    while(1){
        SysCB.SystemTimer = time(NULL);     //记录当前系统时间
        if(SysCB.SystemTimer - SysCB.PingTimer > 60 && clientTx.flag == 0){ //60s发一次ping包，发的时候保证缓存区内没有数据要发送
             SysCB.PingTimer = SysCB.SystemTimer;
             if(SysCB.SysEventFlag&PING_SENT){                     //判断PING_SENT标志，如果置位，进入if，说明上传的PING数据包发送后，没有收到服务器的PINGRESP回复包，可能掉线了
                 printf("PING checks the occurrence of the event...\r\n");                //输出信息
                 MQTT_PingREQ();                                   //将一个PING数据包加入发送缓冲区
                 SysCB.SysEventFlag |= PING_CHECK;                 //表示PING检查事件发生
                 SysCB.PingTimeouter = 0;                          //PING检查事件计数器 从0开始计数
             }else{
                 MQTT_PingREQ();                                   //将一个PING数据包加入发送缓冲区
                 SysCB.SysEventFlag |= PING_SENT;                  //表示PING保活包发送事件发生
             }

         }
         if(SysCB.SysEventFlag&PING_CHECK){                 //判断事件是否发生
             SysCB.PingTimeouter++;                         //PING检查事件超时计数器+1
             if(SysCB.PingTimeouter>5){                    //5s内没有PINGRESP回复包，重启
                 printf("Connection drop!\r\n");           //串口输出信息
                 break;                        //退出循环
             }
             if(clientTx.flag == 0)
                 MQTT_PingREQ();                                   //将一个PING数据包加入发送缓冲区
             sleep(1);
         }


         /*-----------------------------------------------------------------------------------------*/
         /*                             处 理 客 户 端 接 收 到 的 数 据                             */
         /*-----------------------------------------------------------------------------------------*/
         if(clientRx.flag){                                             //如果客户端接收完成
             /*
             printf("客户端本次接收%d字节报文数据：\r\n", clientRx.datalen); //输出提示信息
             for(int i=0; i<clientRx.datalen; i++)
                 printf("%02x ", clientRx.buff[i]);		               //输出提示信息
             printf("\r\n");		                                   //输出提示信息
             */
             passive_event(clientRx.buff, clientRx.datalen);            //处理客户端被动事件数据
             clientRx.flag = 0;         //清除客户端的接收完成标志
         }

         /*----------------------------------------------------------------------------------------*/
         /*                                     主 动 事 件                                        */
         /*----------------------------------------------------------------------------------------*/
         ActiveEvent();
         /*----------------------------------------------------------------------------------------*/
         /*                                        延时                                            */
         /*----------------------------------------------------------------------------------------*/
         usleep(10000);  //延时10ms

     }
}


/**
 ** MqttThread类的定义
 **/
MqttThread::MqttThread(QWidget *parent)
{
    Q_UNUSED(parent);
}

/* 重写run方法，继承QThread的类，只有run方法是在新的线程里 */
void MqttThread::run()
{
    qDebug() << "Start the MQTT iot connection thread...";
    sig_procmask(0);    //添加线程的信号掩码

    mqtt_connection();
}

/**
 ** ServerThread类的定义
 **/
ServerThread::ServerThread(QWidget *parent)
{
    Q_UNUSED(parent);
}

/* 重写run方法，继承QThread的类，只有run方法是在新的线程里 */
void ServerThread::run()
{
    NodeThread *nodeThread;
    qDebug() << "Start the waiting node connection thread...";
    server_init();  //初始化网络服务器
    while(shutdownFlag){

        wait_connect();
        if(ci>=3) continue;
        nodeThread = new NodeThread();  //开启新线程
        nodeThread->ni = ci;
        /* 检查线程是否在运行，如果没有则开始运行 */
        if(!nodeThread->isRunning()) nodeThread->start();

    }


}

/**
 ** NodeThread类的定义
 **/
NodeThread::NodeThread(QWidget *parent)
{
    Q_UNUSED(parent);
}

/* 重写run方法，继承QThread的类，只有run方法是在新的线程里 */
void NodeThread::run()
{
    qDebug() << "Start the node connection thread...";
    sig_procmask(clientConn[ni].sig);
    unsigned int dataUpdataTime = w->currentDateTime;


    PingThread ping;
    ping.ni = ni;   //给ni赋处值
    /* 检查线程是否在运行，如果没有则开始运行ping线程 */
    if(!ping.isRunning()) ping.start();

    int id;
    int bindFlag = 1; //clientConn与节点绑定标志
    while(clientConn[ni].connectFlag){
        // 3s内没有数据则表示掉线
        //if(w->currentDateTime - dataUpdataTime >= 5) break;
        if(clientConn[ni].getDataFlag){
            // 节点更新数据的时间戳
            dataUpdataTime = w->currentDateTime;
            clientConn[ni].getDataFlag = 0;
            // 如果读取到"id"则匹配节点
            if((0 == strncmp("id", clientConn[ni].recvBuf, 2)) && bindFlag) {
                sscanf(clientConn[ni].recvBuf, "id:%d", &id);
                for(nodeIdx=0; nodeIdx<3; nodeIdx++){
                    if(id == w->node[nodeIdx]->id){
                        if(w->node[nodeIdx]->connectFlag){
                            qDebug() << "node"<< nodeIdx+1 <<"is activated repeatedly";
                            nodeIdx = 3;
                            break; //节点已激活，不允许重复激活
                        }
                        w->node[nodeIdx]->connectFlag = clientConn[ni].connectFlag;
                        w->node[nodeIdx]->logFlag = 1;  //该存日志了
                        bindFlag = 0;
                        break;
                    }
                }
                if(nodeIdx>=3) break;
                printf("Node %d goes online...\n", nodeIdx+1);
            }
            // 如果读取到"data"则匹配数据
            if (0 == strncmp("data", clientConn[ni].recvBuf, 4)) {
                if(nodeIdx>=3) break;
                sscanf(clientConn[ni].recvBuf, "data:{Temp:%f,Hum:%d,Illum:%d,Ele:%d,anomaly:%d}",
                       &w->node[nodeIdx]->tData, &w->node[nodeIdx]->hData, &w->node[nodeIdx]->iData,
                       &w->node[nodeIdx]->eData, &w->node[nodeIdx]->anomalyFlag);
                w->node[nodeIdx]->dataFlag = 1;  //表示有新数据
            }
            if (0 == strncmp("shutdown", clientConn[ni].recvBuf, 8)) {
                // 节点掉线
                break;
            }
        }
        QCoreApplication::processEvents(); // 处理未处理的事件
    }
    printf("Node %d while break...\n", nodeIdx+1);
    // 断开连接，清理
    close(clientConn[ni].connfd);
    clientConn[ni].connectFlag = 0; //标志位置1
    if(nodeIdx<3){
        w->node[nodeIdx]->connectFlag = clientConn[ni].connectFlag;
        w->node[nodeIdx]->logFlag = 1;  //该存日志了
    }
    // 在适当的时机，停止线程的执行
    ping.quit(); // 或 thread.exit()
    // 等待线程退出
    ping.wait();
    // 释放线程对象
    ping.deleteLater();
    printf("node shutdown...\n");
    // 在线程运行函数内部释放线程对象
    deleteLater();
}


/**
 ** PingThread类的定义
 **/
PingThread::PingThread(QWidget *parent)
{
    Q_UNUSED(parent);
}

/* 重写run方法，继承QThread的类，只有run方法是在新的线程里 */
void PingThread::run()
{
    qDebug() << "Starting the PING thread...";
    while(clientConn[ni].connectFlag){
        //sleep(5);
        ping(&clientConn[ni]);
    }
}
