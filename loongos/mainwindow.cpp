/******************************************************************
Copyright © Deng Zhimao Co., Ltd. 1990-2021. All rights reserved.
* @projectName   04_qtchart
* @brief         mainwindow.cpp
* @author        Deng Zhimao
* @email         1252699831@qq.com
* @net           www.openedv.com
* @date          2021-03-28
*******************************************************************/
#include "mainwindow.h"
int __counter=0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /* 设置最显示位置与大小 */
    this->setGeometry(0, 0, 960, 540);
    // 界面切换
    stackedWidget = new QStackedWidget(this);
    stackedWidget->setGeometry(10, 40, 940, 485);
    // 添加page1的布局和控件
    page1 = new QWidget();
    // 添加page2的布局和控件
    page2 = new QWidget();
    stackedWidget->addWidget(page1);
    stackedWidget->addWidget(page2);

    /* 实例化关闭按钮对象 */
    QPushButton *closeButton = new QPushButton(this);
    closeButton->setGeometry(0,0,10,10);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    /* 实例化切换按钮对象 */
    QPushButton *swicthButton = new QPushButton(this);
    swicthButton->setGeometry(109,0,742,29);
    swicthButton->setStyleSheet("background-image: url(:/images/button/swicth.png); background-color: transparent;");
    connect(swicthButton, SIGNAL(clicked()), this, SLOT(switch_interface()));

    /* 图片绘制 */
    QPushButton *paintState = new QPushButton(this);
    paintState->setGeometry(20,16,271,14);
    paintState->setStyleSheet("background-image: url(:/images/state.png); background-color: transparent;");

    //创建系统时间
    timeLabel = new QLabel(this);
    QDateTime timeDate = QDateTime::currentDateTime();  // 获取当前时间
    currentDateTime = timeDate.toTime_t();   // 将当前时间转为时间戳
    currentDateTimeString = timeDate.toString("yyyy-MM-dd hh:mm:ss");
    timeLabel->setText(currentDateTimeString);
    timeLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: white; background-color: transparent;");
    timeLabel->setGeometry(815,15,135,12);

    // 数据导出文件提醒
    exportDataLabel = new QLabel(this);
    exportDataLabel->setStyleSheet("font-size: 10px; font-weight: bold; color: rgba(255, 255, 255, 128);; background-color: transparent;");
    exportDataLabel->setGeometry(302,370,380,12);
    exportLabelDisplay = 0; // 为0则不显示


    page1Layout = new QHBoxLayout();
    page2Layout = new QHBoxLayout();

    QFont font; //字体间变量
    for(int i=0; i<3; i++){
        node[i] = new Node();
        vLayout[i] = new QVBoxLayout();

        /* 实例化节点按钮对象 */
        node[i]->nodeNum = i+1;
        node[i]->nodeButton = new QPushButton(this);
        node[i]->nodeButton->setText("节点 " + QString::number(i+1));
        // 设置数据和日志的文件名
        node[i]->dataFileName = QString("node%1DataFile.txt").arg(i+1);
        node[i]->logFileName = QString("node%1LogFile.txt").arg(i+1);

        // 设置颜色
        QPalette palette = node[i]->nodeButton->palette();
        palette.setColor(QPalette::ButtonText, Qt::white);
        node[i]->nodeButton->setPalette(palette);
        // 设置字体大小
        font = node[i]->nodeButton->font();
        font.setPointSize(6); // 设置字体大小为12
        font.setBold(true);    // 设置字体加粗
        node[i]->nodeButton->setFont(font);
        // 设置背景
        node[i]->nodeButton->setStyleSheet("background-image: url(:/images/button/grey.png); background-color: transparent;");
        connect(node[i]->nodeButton, SIGNAL(clicked()), this, SLOT(nodeButtonClick()));


        vLayout[i]->addWidget(node[i]->temperature->chartView);
        vLayout[i]->addWidget(node[i]->humidity->chartView);
        vLayout[i]->addWidget(node[i]->illumination->chartView);
        /* 设置间隔为50symbol */
        vLayout[i]->setSpacing(5);
    }

    node[0]->id = NODE1ID;
    node[1]->id = NODE2ID;
    node[2]->id = NODE3ID;

    nodeSymbol = 0; //显示节点1
    pageSymbol = 0; //显示page1
    set_page1();
    set_page2();
    // 根据指定的QWidget指针切换
    stackedWidget->setCurrentWidget(page1);


    /* 定时器 */
    timer = new QTimer(this);
    /* 定时1s */
    timer->start(500);
    /* 信号槽连接 */
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeOut()));

    /* 设置随机种子，随机数初始化 */
    qsrand(time(0));
}

void MainWindow::set_page1()
{
    /* 设置位置大小 */
    page1->setGeometry(10, 40, 940, 485);
    // 左边视图
    leftWidget = new QWidget(page1);
    leftWidget->setGeometry(0, 35, 250, 450);
    /* 添加曲线图 */
    page1Layout->addLayout(vLayout[nodeSymbol]);
    leftWidget->setLayout(page1Layout);

    // 设置按钮的大小
    node[nodeSymbol]->nodeButton->setGeometry(105,40,60, 30);
    node[nodeSymbol]->nodeButton->show();
    node[(nodeSymbol+1)%3]->nodeButton->hide();
    node[(nodeSymbol+2)%3]->nodeButton->hide();

    // 中间视图
    middleWidget = new QWidget(page1);
    middleWidget->setGeometry(260, 0, 420, 485);
    // 按键视图
    QWidget *buttonWidget = new QWidget(middleWidget);
    buttonWidget->setGeometry(12, 2, 386, 138);
    buttonWidget->setStyleSheet("background-image: url(:/images/data.png); background-color: transparent;");
    for(int i=0; i<3; i++){
        node[i]->exportButton = new QPushButton(middleWidget);
        node[i]->exportButton->setGeometry(12+i*132,11,40,25);
        node[i]->exportButton->setStyleSheet("background-image: url(:/images/transparency.png); background-color: transparent;");
        // 导出按钮与槽函数连接
        connect(node[i]->exportButton, SIGNAL(clicked()), this, SLOT(exportButtonClick()));

        //创建t数据
        node[i]->tLabel = new QLabel(middleWidget);
        node[i]->tLabel->setText(QString("%1℃").arg(node[i]->tData));
        node[i]->tLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: yellow; background-color: transparent;");
        node[i]->tLabel->setGeometry(72+i*132,32,60,35);
        //创建h数据
        node[i]->hLabel = new QLabel(middleWidget);
        node[i]->hLabel->setText(QString("%1%").arg(node[i]->hData));
        node[i]->hLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: blue; background-color: transparent;");
        node[i]->hLabel->setGeometry(72+i*132,32+24,60,35);
        //创建i数据
        node[i]->iLabel = new QLabel(middleWidget);
        node[i]->iLabel->setText(QString("%1L").arg(node[i]->iData));
        node[i]->iLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: cyan; background-color: transparent;");
        node[i]->iLabel->setGeometry(72+i*132,32+24+24,60,35);
        //创建e数据
        node[i]->eLabel = new QLabel(middleWidget);
        node[i]->eLabel->setText(QString("%1%").arg(node[i]->eData));
        node[i]->eLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: rgba(0, 255, 0, 255); background-color: transparent;");
        node[i]->eLabel->setGeometry(72+i*132,32+24+24+24,60,35);
    }


    /* 图片绘制 */
    okWidget = new QWidget(middleWidget);
    okWidget->setGeometry(10,162,400,195);
    okWidget->setStyleSheet("background-image: url(:/images/ok.png); background-color: transparent;");
    /* 图片绘制 */
    QPushButton *paintState = new QPushButton(middleWidget);
    paintState->setGeometry(42,360,334,116);
    paintState->setStyleSheet("background-image: url(:/images/loongos.png); background-color: transparent;");

    // 右边试图
    rightWidget = new QWidget(page1);
    rightWidget->setGeometry(690, 0, 250, 485);
    /* 图片绘制 */
    QPushButton *paintText = new QPushButton(rightWidget);
    paintText->setGeometry(0,35,250,450);
    paintText->setStyleSheet("background-image: url(:/images/text.png); background-color: transparent;");
    /* 布局设置 */

    Node::textEdit = new QTextEdit(rightWidget);
    Node::textEdit->setReadOnly(true);
    Node::textEdit->setGeometry(10,45,230,430);
    Node::textEdit->setStyleSheet("font-size: 15px; color: white;  background-color: transparent;");

    // 实现一个下拉菜单
    QComboBox *comboBox = new QComboBox(rightWidget);
    comboBox->setGeometry(62,0,125,25);
    comboBox->setStyleSheet("font-size: 10px; font-weight: bold; color: white; background-color: blue; selection-background-color: gray; selection-color: white;");
    comboBox->addItem("    查看系统日志");
    for(int i=0; i<3; i++){
        comboBox->addItem(QString("    查看节点 %1 日志").arg(i+1));
    }
    /* 信号槽连接 */
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxIndexChanged(int)));

}

void MainWindow::set_page2()
{
    /* 设置位置大小 */
    page2->setGeometry(10, 40, 940, 485);
    chartWidget = new QWidget(page2);
    chartWidget->setGeometry(0, 35, 940, 450);


    /* 将主布局设置为widget的布局 */
    chartWidget->setLayout(page2Layout);

}

void MainWindow::nodeButtonClick()
{
    if(!pageSymbol){ //当前界面是page1
        // 剔除之前的
        page1Layout->removeItem(vLayout[nodeSymbol]);
        // 轮拨
        nodeSymbol = (nodeSymbol+1)%3;
        // 显示其中一个节点，隐藏其他节点
        node[nodeSymbol]->nodeButton->setGeometry(105,40,60, 30);
        node[nodeSymbol]->nodeButton->show();
        node[(nodeSymbol+1)%3]->nodeButton->hide();
        node[(nodeSymbol+2)%3]->nodeButton->hide();
        /* 白痴QT非要这样才能显示？ */
        page2Layout->addLayout(vLayout[nodeSymbol]);
        page2Layout->removeItem(vLayout[nodeSymbol]);
        // 添加新页
        page1Layout->addLayout(vLayout[nodeSymbol]);
    }else{ //当前界面是page2
        // 获取发送信号的对象,判断谁按的
        QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
        if(senderButton){
            // 根据发送者的不同做出相应的处理
            if (senderButton->text() == "节点 1") nodeSymbol = 0;
            else if (senderButton->text() == "节点 2") nodeSymbol = 1;
            else if (senderButton->text() == "节点 3") nodeSymbol = 2;

            switch_interface();
        }
    }

}

void MainWindow::sendMails(int &mailSendSym, int flag, const char* subject)
{
    mailSendSym += 1;
    if(mailSendSym == 1){
        bSMTP smtp;
        textMail = QString("%1 :\n").arg(subject);
        for(int i=0; i<3; i++){
            if(flag & (1<<i)) textMail += QString("Node %1\n").arg(i+1);
        }
        textMail += currentDateTimeString;
        // 邮件发送成功返回0
        if(smtp.sendMails(qPrintable(toMail), "分布式工业无线物联网传感系统", qPrintable(textMail))){
            // 发送失败，下次再次尝试发送
            mailSendSym = 0;
            qDebug() << "Email sending failure...";
        }else qDebug() << "Email sent successfully";
    }
    if(mailSendSym >= 30000) mailSendSym = 1; // 防止数据溢出
}

void MainWindow::timerTimeOut()
{
    QDateTime timeDate = QDateTime::currentDateTime();  // 获取当前时间
    currentDateTime = timeDate.toTime_t();   // 将当前时间转为时间戳
    currentDateTimeString = timeDate.toString("yyyy-MM-dd hh:mm:ss");
    ++__counter;
    // qDebug() << "trigger time out " << ++__counter << endl;     // antimo: timeout debug

    timeLabel->setText(currentDateTimeString);
    /* 更新节点状态 */
    int systemAnomalyFlag = 0;
    int lowEleFlag = 0;
    int dropLineFlag = 0;
    for(int i=0; i<3; i++){
        // 更新节点数据
        if(node[i]->dataFlag){      
            // qDebug() << " excuting node " << i << ", enter updateData()" << endl;    // antimo: updateData debug
            node[i]->dataFlag = 0;
            node[i]->updateData(i);
            node[i]->dataFlag = 1;      // antimo: I added this line
        }
        if(node[i]->connectFlag){
            if(node[i]->anomalyFlag){   //异常优先设置成异常
                node[i]->set_anomaly();
                systemAnomalyFlag += 1<<i; // 异常节点统计
            }
            else if(node[i]->lowEleFlag){   //其次是电量低
                node[i]->set_low_ele();
                lowEleFlag += 1<<i; // 低电量节点统计
            }else node[i]->set_online();

            if(node[i]->logFlag){
                node[i]->log = QString("%1 Node %2 goes online").arg(currentDateTimeString).arg(i+1);
                node[i]->exportLog();
            }

        }else{
            node[i]->set_drop_line();
            dropLineFlag += 1<<i;   // 不在线节点统计

            if(node[i]->logFlag){
                node[i]->log = QString("%1 Node %2 dropped").arg(currentDateTimeString).arg(i+1);
                node[i]->exportLog();
            }
        }
    }

    if(systemAnomalyFlag){
        // 异常提醒
        okWidget->setStyleSheet("background-image: url(:/images/notok.png); background-color: transparent;");
        sendMails(anomalyMailSendSym, systemAnomalyFlag, "Node Anomaly");
    }else{
        okWidget->setStyleSheet("background-image: url(:/images/ok.png); background-color: transparent;");
        anomalyMailSendSym = 0;
    }
    // 发送低电量邮件
    if(lowEleFlag){
        sendMails(lowEleMailSendSym, lowEleFlag, "Low Node Power");
    }else{
        lowEleMailSendSym = 0;
    }

    // 发送掉线邮件
    if(dropLineFlag){
        sendMails(dropLineMailSendSym, dropLineFlag, "Node Dropped");
    }else{
        dropLineMailSendSym = 0;
    }

    if(exportLabelDisplay){
        exportLabelDisplay++;
        if(exportLabelDisplay > 3){
            exportLabelDisplay = 0; // 清0
            exportDataLabel->hide(); // 不显示
        }
    }
}

void MainWindow::switch_interface()
{
    if(!pageSymbol){
        page1Layout->removeItem(vLayout[nodeSymbol]);
        for(int i=0; i<3; i++){
            // 设置按钮的大小
            node[i]->nodeButton->setGeometry(131+i*319,40,60, 30);
            node[i]->nodeButton->show();
            page2Layout->addLayout(vLayout[i]);
        }
        page2Layout->setSpacing(10);
        stackedWidget->setCurrentWidget(page2);
        pageSymbol = !pageSymbol;
    }
    else{
        // 显示其中一个节点，隐藏其他节点
        node[nodeSymbol]->nodeButton->setGeometry(105,40,60, 30);
        node[nodeSymbol]->nodeButton->show();
        node[(nodeSymbol+1)%3]->nodeButton->hide();
        node[(nodeSymbol+2)%3]->nodeButton->hide();
        for(int i=0; i<3; i++){
            // 从2页中删除
            page2Layout->removeItem(vLayout[i]);
        }
        /* 白痴QT非要这样才能显示？ */
        page2Layout->addLayout(vLayout[nodeSymbol]);
        page2Layout->removeItem(vLayout[nodeSymbol]);
        // 添加新页
        page1Layout->addLayout(vLayout[nodeSymbol]);
        stackedWidget->setCurrentWidget(page1);
        pageSymbol = !pageSymbol;
    }
}

void MainWindow::comboBoxIndexChanged(int index)
{
    Node::textEdit->clear();
    Node::comboBoxIndex = index;
    QString fileName;
    switch(index) {
        case 0: fileName = "查看系统日志文件"; break;
        case 1: case 2: case 3:
            fileName.sprintf("查看节点 %d 日志文件", index); break;
        default: return;
    }
    Node::textEdit->append(fileName);
    if(index){
        QFile file(node[index-1]->logFileName);
        // 判断文件是否存在
        if (!file.exists()) return;
        // 以只读的方式打开
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
        // 读取文本到textEdit
        Node::textEdit->setPlainText(file.readAll().trimmed());
        file.close();
    }
}

void MainWindow::exportButtonClick()
{
    // 获取发送信号的对象,判断谁按的
    QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
    // 计算位置对于的按键
    int i = senderButton->pos().x()/132;
    node[i]->exportData();
    exportDataLabel->setText(QString("The data of node %1 has been exported to file node%2DataFile.txt").arg(i+1).arg(i+1));
    exportDataLabel->show();
    exportLabelDisplay = 1; // 0< <4显示
}

MainWindow::~MainWindow()
{
}

Axis::Axis()
{
    axis = new QValueAxis();
}
Axis::Axis(int minVTemp, int maxVTemp, QString labelFormat, QString titleText)
{
    axis = new QValueAxis();
    /* 坐标的范围 */
    minV = minVTemp;
    maxV = maxVTemp;
    /* 设置显示格式 */
    axis->setLabelFormat(labelFormat);
    /* y轴标题 */
    axis->setTitleText(titleText);
    axis->setTitleFont(QFont("Calibri", 6, QFont::Light));
    axis->setTitleBrush(QBrush(Qt::white));
    axis->setLabelsColor(Qt::white);
    set_axis();
}

void Axis::set_axis()
{
    /* 设置坐标轴范围 */
    axis->setRange(minV, maxV);
}

void Axis::update_axis()
{
    minV += 3;
    maxV += 3;
    /* 设置更新坐标轴范围 */
    axis->setRange(minV, maxV);
}

Axis::~Axis()
{
    delete axis;
}

ChartClass::ChartClass(Axis *axisXTemp, Axis *axisYTemp, QString title, char type)
{
    /* 图表视图实例化 */
    chartView = new QChartView();
    /* 图表实例化 */
    chart = new QChart();
    /* splineSeries曲线实例化（折线用QLineSeries） */
    splineSeries = new QSplineSeries();
    /* 修改曲线颜色 */
    QPen pen = splineSeries->pen();
    switch(type) {
        case 't': pen.setColor(Qt::yellow); break; // 设置为黄色
        case 'h': pen.setColor(Qt::blue); break; // 设置为蓝色
        case 'i': pen.setColor(Qt::cyan); break; // 设置为绿色
    }
    pen.setWidth(1); // 设置线条宽度为3
    splineSeries->setPen(pen);

    /* y轴 */
    axisY = axisYTemp;
    /* x轴 */
    axisX = axisXTemp;

    /* legend译图例类型，以绘图的颜色区分，本例设置为隐藏 */
    chart->legend()->hide();
    /* chart设置标题 */
    chart->setTitle(title);
    chart->setTitleFont(QFont("Calibri", 6, QFont::Light));

    chart->setTitleBrush(QBrush(Qt::white));
    /* 添加一条曲线splineSeries */
    chart->addSeries(splineSeries);


    /* y轴标题位置（设置坐标轴的方向） */
    chart->addAxis(axisY->axis, Qt::AlignLeft);
    /* x轴标题位置（设置坐标轴的方向） */
    chart->addAxis(axisX->axis, Qt::AlignBottom);
    //取消x轴的网格线
    axisX->axis->setGridLineVisible(false);
    //chart->axisY()->setGridLineVisible(false);

    /* 设置曲线图背景透明 */
    chart->setBackgroundVisible(false);
    chartView->setStyleSheet("border-image:url(:/images/chart.png)");


    /* 设置绘图边界 */
    chart->setMargins(QMargins(0, 0, 0, 0));

    /* 将splineSeries附加于y轴上 */
    splineSeries->attachAxis(axisY->axis);
    /* 将splineSeries附加于x轴上 */
    splineSeries->attachAxis(axisX->axis);

    /* 将图表的内容设置在图表视图上 */
    chartView->setChart(chart);
    /* 设置抗锯齿 */
    chartView->setRenderHint(QPainter::Antialiasing);
}


ChartClass::~ChartClass()
{
    delete chartView;
    delete chart;
    delete splineSeries;
    delete axisY;
    delete axisX;
}



Node::Node()
{
    //表示一开始无数据
    tData = 125;
    hData = 100;
    iData = 65535;
    eData = 100;
    dataFlag = 1;           // antimo: originally 0, hard to understand why so.
    mqttDataFlag = 0;
    /* 数据最大个数 */
    maxSize = 50;
    timeLabel = new QLabel();
    dataNum = 0;

    /* 温度视图创建 */
    Axis *axisXTemp = new Axis(0, 5000, "%i", "时间/ms");
    Axis *axisYTemp = new Axis(0, 75, "%i", "温度/℃");
    temperature = new ChartClass(axisXTemp, axisYTemp, "温度视图", 't');

    /* 湿度视图创建 */
    axisXTemp = new Axis(0, 5000, "%i", "时间/ms");
    axisYTemp = new Axis(0, 100, "%i", "湿度/RH");
    humidity = new ChartClass(axisXTemp, axisYTemp, "湿度视图", 'h');

    /* 光照强度视图创建 */
    axisXTemp = new Axis(0, 5000, "%i", "时间/ms");
    axisYTemp = new Axis(0, 65535, "%i", "光照强度/Lux");
    illumination = new ChartClass(axisXTemp, axisYTemp, "光照强度视图", 'i');

}

void Node::receivedData(ChartClass *attribute, int value)
{
    /* 将数据添加到data中 */
    attribute->data.append(value);

    /* 当储存数据的个数大于最大值时，把第一个数据删除 */
    if(attribute->data.size() > maxSize) {
        /* 移除data中第一个数据 */
        attribute->data.removeFirst();
        attribute->axisX->update_axis();
    }

    /* 先清空 */
    attribute->splineSeries->clear();

    /* 计算x轴上的点与点之间显示的间距 */
    int xSpace = (attribute->axisX->maxV - attribute->axisX->minV) / (maxSize);

    /* 添加点，xSpace * i 表示第i个点的x轴的位置 */
    for (int i = 0; i < attribute->data.size(); ++i) {
        attribute->splineSeries->append(attribute->axisX->minV + xSpace * i, attribute->data.at(i));
    }
}

void Node::exportData()
{
    QFile file(dataFileName);
    /* 文件不存在则创建文件并打开，存在则直接打开 */
    if (!file.exists()) file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    else file.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&file);
    // 将数据加载进文件
    for(int i=0; i<dataNum; i++) out << dataBuf[i] << endl;
    // 数据缓冲区清空
    dataNum = 0;
    file.close();
}

void Node::exportLog()
{
    QFile file(logFileName);
    /* 文件不存在则创建文件并打开，存在则直接打开 */
    if (!file.exists()) file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    else file.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&file);
    // 将数据加载进文件
    out << log << endl;
    file.close();
    if(comboBoxIndex == 0 || comboBoxIndex == nodeNum) textEdit->append(log);
    logFlag = 0;
}

void Node::updateData(int bango)
{
    reveiceDateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    /* 设置随机数*/
    
    tData = (__counter>=50&&bango==0?70.5:25.5) + qrand() % 10;
    hData = 62 + qrand() % 4;
    iData = std::min((__counter>=50&&bango==0?50000:10000) + (qrand() % 7192)*2, 65535);
    eData = 100;
    // antimo: check rand
    // qDebug() << tData << " " << hData << " " << iData << " " << eData << endl;
    

    mqttDataFlag = 1; 
    if(eData<20) lowEleFlag = 1; //电量低于20%提示电量低
    else lowEleFlag = 0;

    tLabel->setText(QString("%1℃").arg(tData));
    hLabel->setText(QString("%1%").arg(hData));
    iLabel->setText(QString("%1L").arg(iData));
    eLabel->setText(QString("%1%").arg(eData));
    // 格式化显示和导出格式
    dataBuf[dataNum].sprintf("%s Temp:%0.2f Hum:%d Illum:%d", qPrintable(reveiceDateTimeString), tData, hData, iData);
    if(comboBoxIndex == 0) textEdit->append(dataBuf[dataNum]);
    dataNum++;
    // 数据已满
    if(dataNum >= DATABUFMAX) exportData();

    receivedData(temperature, tData);
    receivedData(humidity, hData);
    receivedData(illumination, iData);


}

void Node::set_online()
{
    nodeButton->setStyleSheet("background-image: url(:/images/button/blue.png); background-color: transparent;");
}

void Node::set_low_ele()
{
    nodeButton->setStyleSheet("background-image: url(:/images/button/yellow.png); background-color: transparent;");
}

void Node::set_anomaly()
{
    nodeButton->setStyleSheet("background-image: url(:/images/button/red.png); background-color: transparent;");
}

void Node::set_drop_line()
{
    nodeButton->setStyleSheet("background-image: url(:/images/button/blue.png); background-color: transparent;");
    // antimo: FAKE DATA!!!!!! should be grey
}

Node::~Node()
{
    delete nodeButton;
    delete exportButton;
    delete tLabel;
    delete hLabel;
    delete iLabel;
    delete eLabel;
    delete temperature;
    delete humidity;
    delete illumination;
}
/*
to do:
[x] node change to blue
fog
title tag
car: find code


for senpai:
why segmentation fault




*/