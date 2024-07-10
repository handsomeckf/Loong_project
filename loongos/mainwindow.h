/******************************************************************
Copyright © Deng Zhimao Co., Ltd. 1990-2021. All rights reserved.
* @projectName   04_qtchart
* @brief         mainwindow.h
* @author        Deng Zhimao
* @email         1252699831@qq.com
* @net           www.openedv.com
* @date          2021-03-28
*******************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QApplication>
#include <QChartView>
#include <QSplineSeries>
#include <QScatterSeries>
#include <QDebug>
#include <QValueAxis>
#include <QTimer>
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include <QDateTime>
#include "smtp.h"
#include <random>

extern "C" {
#include "server.h"
}
/*  必需添加命名空间 */
QT_CHARTS_USE_NAMESPACE
#define DATABUFMAX 30

#define NODE1ID  1234
#define NODE2ID  1235
#define NODE3ID  1236

class Axis
{
public:
    Axis();
    Axis(int minVTemp, int maxVTemp, QString labelFormat, QString titleText);
    ~Axis();
    int minV;
    int maxV;
    QValueAxis *axis;
    void set_axis();
    void update_axis();
};

class ChartClass
{
public:
    ChartClass(Axis *axisXTemp, Axis *axisYTemp, QString title, char type);
    ~ChartClass();

    /* QList int类型容器 */
    QList<int> data;

    /* 图表视图 */
    QChartView *chartView;
    /* 图表 */
    QChart *chart;
    /* splineSeries曲线（折线用QLineSeries） */
    QSplineSeries *splineSeries;
    /* y轴 */
    Axis *axisY;
    /* x轴 */
    Axis *axisX;


};
/* 节点类，一个节点里3张曲线图 */
class Node
{
public:
    Node();
    ~Node();
    /* 数据最大个数 */
    int maxSize;
    // 创建节点按钮
    QPushButton *nodeButton;
    QPushButton *exportButton;

    float tData;
    int hData;
    int iData;
    int eData;
    int anomalyFlag = 0;
    int lowEleFlag = 0;
    int id;
    int nodeNum;

    /* 网络连接部分 */
    int connectFlag = 0;
    int mqttDataFlag = 0;   //上传mqtt
    int dataFlag = 0;  //有数据来
    int logFlag = 0; //1表示该存储日志了

    QString dataFileName;
    QString logFileName;
    QString dataBuf[DATABUFMAX];
    QString log;
    int dataNum;

    QLabel *tLabel;
    QLabel *hLabel;
    QLabel *iLabel;
    QLabel *eLabel;
    // 静态成员变量，共用一个内存
    static int comboBoxIndex;
    static QString reveiceDateTimeString;
    static QLabel* timeLabel;
    static QTextEdit *textEdit;
    /* 监测数据 */
    ChartClass *temperature; //温度
    ChartClass *humidity;  //湿度
    ChartClass *illumination;  //光照
    /* 接收数据接口 */
    void receivedData(ChartClass *attribute, int value);
    void exportData();
    void exportLog();
    void updateData(int bango);

    void set_online();
    void set_low_ele();
    void set_anomaly();
    void set_drop_line();
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    /* 创建3个设备节点 */
    QString toMail;
    QString textMail;
    Node *node[3];
    unsigned int currentDateTime;
    ~MainWindow();

private:
    QString currentDateTimeString;
    QLabel *timeLabel;

    QLabel *exportDataLabel;
    int exportLabelDisplay;  // 导出提示文字显示标志，0< <4显示，控制显示的时间


    int nodeSymbol;
    int anomalyMailSendSym = 0;
    int lowEleMailSendSym = 0;
    int dropLineMailSendSym = 0;

    QStackedWidget *stackedWidget;
    bool pageSymbol;
    /* 先创建两个布局作为两个界面的主布局 */
    QHBoxLayout *page1Layout; //3个水平表示3个节点
    QHBoxLayout *page2Layout; //3个水平表示3个节点
    QVBoxLayout *vLayout[3];

    QWidget *page1;
    // 曲线图视图
    QWidget *leftWidget;
    // 按键视图
    QWidget *middleWidget;
    QWidget *okWidget;
    // 日志试图
    QWidget *rightWidget;
    QFile file;

    QWidget *page2;
    QWidget *chartWidget;

    /* 定时器 */
    QTimer *timer;

    void set_page1();
    void set_page2();    
    void sendMails(int &mailSendSym, int flag, const char* subject);
public slots:                       /*antimo: originally private*/
    void nodeButtonClick();
    void timerTimeOut();
    void switch_interface();
    void comboBoxIndexChanged(int index);
    void exportButtonClick();
};

#endif // MAINWINDOW_H
