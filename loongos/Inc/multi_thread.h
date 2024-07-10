#ifndef MULTI_THREAD_H
#define MULTI_THREAD_H
#include "mainwindow.h"
#include <QThread>
#include <QDebug>

extern "C" {
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <termios.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <signal.h>
    #include <string.h>
    #include <time.h>
    #include "client.h"
    #include "server.h"
}

class MqttThread : public QThread
{
public:
    MqttThread(QWidget *parent = nullptr);
    /* 重写run方法，继承QThread的类，只有run方法是在新的线程里 */
    void run() override;
};

class ServerThread : public QThread
{
public:
    ServerThread(QWidget *parent = nullptr);
    /* 重写run方法，继承QThread的类，只有run方法是在新的线程里 */
    void run() override;
};

class NodeThread : public QThread
{
public:
    NodeThread(QWidget *parent = nullptr);
    unsigned int dataUpdataTime;
    int ni;
    int nodeIdx = 3;
    /* 重写run方法，继承QThread的类，只有run方法是在新的线程里 */
    void run() override;
};


class PingThread : public QThread
{
public:
    PingThread(QWidget *parent = nullptr);
    int ni;
    /* 重写run方法，继承QThread的类，只有run方法是在新的线程里 */
    void run() override;
};

extern MainWindow *w;
#endif // MULTI_THREAD_H
