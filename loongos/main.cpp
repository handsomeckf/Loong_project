#include "multi_thread.h"
#include <QApplication>



QTextEdit* Node::textEdit = nullptr;
QString Node::reveiceDateTimeString;
int Node::comboBoxIndex;
QLabel* Node::timeLabel = nullptr;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    w = new MainWindow();

    w->toMail = QString("17313195440@163.com"); //设置默认邮箱
    if(argc > 1 && QString(argv[1]).contains('@')){ // 输入的第二个参数是否是一个邮箱
        w->toMail = QString(argv[1]);
        qDebug() << "The default email address is set to" << w->toMail;
    }
    else{
        qDebug() << "Usage: "<< argv[0] << " Email address";  //打印帮助信息
    }

    // 设置窗口标志，去除系统边框
    // w->setWindowFlags(Qt::FramelessWindowHint);
    w->setStyleSheet("background-color: #00002D;");
    w->show();

    // MqttThread *workerThread = new MqttThread();         // antimo: 30~37, 41, test only GUI, without threads; successful
    // /* 检查线程是否在运行，如果没有则开始运行 */
    // if(!workerThread->isRunning()) workerThread->start();

    // ServerThread *serverThread = new ServerThread();
    // /* 检查线程是否在运行，如果没有则开始运行 */
    // if(!serverThread->isRunning()) serverThread->start();

    int result = a.exec();
    delete w;
    // delete workerThread;
    return result;
}

