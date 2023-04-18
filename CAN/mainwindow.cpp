#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QMessageBox>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QStringList listHeader;
    listHeader << "系统时间" << "时间标识" << "时间标识(s)" << "CAN通道" << "传输方向" << "ID号" << "帧类型" << "帧格式" << "长度" << "数据";

    ui->comboBox_3->setCurrentIndex(10);

    ui->tableWidget->setColumnCount(listHeader.count());
    ui->tableWidget->setHorizontalHeaderLabels(listHeader);

    ui->tableWidget->setColumnWidth(0,120);
    ui->tableWidget->setColumnWidth(1,120);
    ui->tableWidget->setColumnWidth(2,120);
    ui->tableWidget->setColumnWidth(3,100);
    ui->tableWidget->setColumnWidth(4,80);
    ui->tableWidget->setColumnWidth(5,120);
    ui->tableWidget->setColumnWidth(6,90);
    ui->tableWidget->setColumnWidth(7,90);
    ui->tableWidget->setColumnWidth(8,80);
    ui->tableWidget->setColumnWidth(9,250);

    //for(int i = 0;i < 9;i ++)
    //    ui->tableWidget->horizontalHeader()->setSectionResizeMode(i,QHeaderView::Stretch);


    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    canthread = new CANThread();
    connect(canthread,&CANThread::getProtocolData,this,&MainWindow::onGetProtocolData);
    connect(canthread,&CANThread::boardInfo,this,&MainWindow::onBoardInfo);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::AddDataToList(QStringList strList)
{
    if(!ui->checkBox_4->checkState())
        return;
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    for(int i = 0; i < strList.count();i ++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(strList.at(i),0);
        ui->tableWidget->setItem(row, i, item);
        if(i != strList.count() - 1)
            ui->tableWidget->item(row,i)->setTextAlignment(Qt::AlignCenter | Qt::AlignHCenter);

    }
    ui->tableWidget->scrollToBottom();
}
void MainWindow::onGetProtocolData(VCI_CAN_OBJ *vci,unsigned int dwRel,unsigned int channel)
{
    QStringList messageList;
    QString str;
    for(unsigned int i = 0;i < dwRel;i ++)
    {
        messageList.clear();
        messageList << QTime::currentTime().toString("hh:mm:ss zzz");//时间
        if(vci[i].TimeFlag == 1)
        {
            messageList << "0x" + QString("%1").arg(vci[i].TimeStamp,8,16,QChar('0')).toUpper();//时间
            messageList << QString("%1").arg(QVariant(vci[i].TimeStamp * 0.1).toUInt(),8,10,QChar('0'));//时间
        }
        else
        {
            messageList << "无";//时间
            messageList << "无";//时间
        }
        messageList << "CH" + QString::number(channel);
        messageList << "接收";//收发
        messageList << "0x" + QString("%1").arg(vci[i].ID,8,16,QChar('0')).toUpper();//ID
        messageList << ((vci[i].RemoteFlag == 1) ? "远程帧" : "数据帧");//类型
        messageList << ((vci[i].ExternFlag == 1) ? "扩展帧" : "标准帧");//Frame
        QString str = "";
        if(vci[i].RemoteFlag == 0)//数据帧显示数据
        {
            messageList << "0x" + QString::number(vci[i].DataLen,16).toUpper();//长度
            str = "x| ";
            for(int j = 0;j < vci[i].DataLen;j ++)
                str += QString("%1 ").arg(vci[i].Data[j],2,16,QChar('0')).toUpper();//QString::number(recvCANData[i].frame.data[j],16) + " ";
        }
        else
            messageList << "0x0";//长度
        messageList << str;//数据
        /*
        //listHeader << "时间"  << "收/发" << "ID" << "Frame" << "类型" << "长度" << "数据";
        messageList.clear();
        messageList << QString::number(channel);
        messageList << "收";//收发
        messageList << "0x" + QString("%1").arg(vci[i].ID,8,16,QChar('0'));//ID
        messageList << ((vci[i].ExternFlag == 1) ? "扩展帧" : "标准帧");//Frame
        messageList << ((vci[i].RemoteFlag == 1) ? "远程帧" : "数据帧");//类型
        str = "";
        if(vci[i].RemoteFlag == 0)//数据帧显示数据
        {
            messageList << QString::number(vci[i].DataLen);//长度
            for(int j = 0;j < vci[i].DataLen;j ++)
                str += QString("%1 ").arg(vci[i].Data[j],2,16,QChar('0'));//QString::number(recvCANData[i].frame.data[j],16) + " ";
        }
        else
            messageList << "0";//长度
        messageList << str;//数据
        */
        AddDataToList(messageList);
    }
}

QString versionStr(USHORT ver)
{
    return "V" + QString::number((ver & 0x0FFF) /0x100,16).toUpper() + "." + QString("%1 ").arg((ver & 0x0FFF) % 0x100,2,16,QChar('0')).toUpper();
}

void MainWindow::onBoardInfo(VCI_BOARD_INFO vbi)
{
    ui->label_9->setText(QString::number(vbi.can_Num));
    ui->label_10->setText(versionStr(vbi.hw_Version));
    ui->label_12->setText(versionStr(vbi.in_Version));
    ui->label_13->setText(QString::number(vbi.irq_Num));
}

void MainWindow::on_pushButton_clicked()//打开
{
    if(ui->pushButton->text() == tr("打开设备"))
    {
        UINT baundRate = 0;
        if(ui->comboBox_3->currentText().indexOf("Kbps") != -1)
            baundRate = ui->comboBox_3->currentText().remove("Kbps").toUInt();
        else
            baundRate = QVariant(ui->comboBox_3->currentText().remove("Mbps").toFloat()).toUInt();
        bool dev = canthread->openDevice(4,//QVariant(ui->comboBox->currentIndex()).toUInt(),
                                      QVariant(ui->comboBox_2->currentIndex()).toUInt(),
                                      baundRate);
        if(dev == true)
        {
            ui->comboBox->setEnabled(false);
            ui->comboBox_2->setEnabled(false);
            ui->comboBox_3->setEnabled(false);
            ui->pushButton_2->setEnabled(true);
            ui->pushButton->setText(tr("关闭设备"));
        }
        else
            QMessageBox::warning(this,"警告","打开设备失败！");
    }
    else if(ui->pushButton->text() == tr("关闭设备"))
    {
        ui->comboBox->setEnabled(true);
        ui->comboBox_2->setEnabled(true);
        ui->comboBox_3->setEnabled(true);
        ui->comboBox_4->setEnabled(true);
        ui->sendBtn->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
        ui->pushButton_3->setEnabled(false);
        ui->pushButton_4->setEnabled(false);
        ui->pushButton->setText(tr("打开设备"));
        canthread->stop();
        canthread->closeDevice();
    }
}

void MainWindow::on_sendBtn_clicked()
{
//    if(ui->comboBox_6->currentIndex() == 0)//标准帧，ID 范围0-0x7FF
//    {
//        if(ui->sendIDEdit->text().toInt(Q_NULLPTR,16) > 0x7FF)
//        {
//            QMessageBox::warning(this,"警告","发送失败，标准帧ID范围为0~0x7FF！");
//            return;
//        }
//    }
//    else
//    {
//        if(ui->sendIDEdit->text().toInt(Q_NULLPTR,16) > 0x1FFFFFFF)
//        {
//            QMessageBox::warning(this,"警告","发送失败，扩展帧ID范围为0~0x1FFFFFFF！");
//            return;
//        }
//    }
    ui->sendIDEdit->setText("000000FF");
    QStringList strList = ui->sendDataEdit->text().split(" ");
    unsigned char data[8];
    memset(data,0,8);
    UINT dlc = 0;
    dlc = strList.count() > 8 ? 8 : strList.count();
    for(int i = 0;i < dlc;i ++)
        data[i] = strList.at(i).toInt(Q_NULLPTR,16);
    if(canthread->sendData(QVariant(ui->comboBox_4->currentIndex()).toUInt(),
                           QVariant(ui->sendIDEdit->text().toInt(Q_NULLPTR,16)).toUInt(),
                           ui->comboBox_5->currentIndex(),
                           ui->comboBox_6->currentIndex(),
                           data,dlc))
    {//发送成功，打印数据
        QStringList messageList;

        messageList.clear();
        messageList << QTime::currentTime().toString("hh:mm:ss zzz");//时间
        messageList << "无";//时间
        messageList << "无";//时间
        messageList << "CH" + QString::number(QVariant(ui->comboBox_4->currentIndex()).toUInt());
        messageList << "发送";//收发
        messageList << "0x" + ui->sendIDEdit->text().toUpper();//ID
        messageList << ((ui->comboBox_5->currentIndex() == 0) ? "数据帧" : "远程帧");//类型
        messageList << ((ui->comboBox_6->currentIndex() == 0) ? "标准帧" : "扩展帧");//Frame
        QString str = "";
        if(ui->comboBox_5->currentIndex() == 0)//数据帧显示数据
        {
            messageList << "0x" + QString::number(dlc,16).toUpper();//长度
            str = "x| ";
            for(int j = 0;j < dlc;j ++)
                str += QString("%1 ").arg((unsigned char)data[j],2,16,QChar('0')).toUpper();//QString::number((unsigned char)data[j],16) + " ";
        }
        else
            messageList << "0x0";//长度
        messageList << str;//数据
        AddDataToList(messageList);
    }
    else
        QMessageBox::warning(this,"警告","数据发送失败！");
}

void MainWindow::on_cleanListBtn_clicked()
{
    ui->tableWidget->setRowCount(0);
}

void MainWindow::on_pushButton_4_clicked()//复位
{
    if(canthread->reSetCAN())
    {
        ui->pushButton_4->setEnabled(false);
        ui->sendBtn->setEnabled(false);
        ui->pushButton_3->setEnabled(true);
    }
    else
        QMessageBox::warning(this,"警告","CAN复位失败！");
}

void MainWindow::on_pushButton_2_clicked()//初始化
{
    if(canthread->initCAN())
    {
        ui->pushButton_3->setEnabled(true);
        ui->pushButton_2->setEnabled(false);
    }
    else
        QMessageBox::warning(this,"警告","CAN初始化失败！");
}

void MainWindow::on_pushButton_3_clicked()//启动
{
    if(canthread->startCAN())
    {
        ui->pushButton_3->setEnabled(false);
        ui->pushButton_4->setEnabled(true);
        ui->sendBtn->setEnabled(true);
        ui->DC_Open_btn->setEnabled(true);
        canthread->start();
    }
    else
        QMessageBox::warning(this,"警告","CAN启动失败！");
}




void MainWindow::on_DC_Open_btn_clicked()
{

    ui->sendDataEdit->setText("00 10 3A 01 00 00 00 00");
    QStringList strList = ui->sendDataEdit->text().split(" ");
    unsigned char data[8];
    memset(data,0,8);
    UINT dlc = 0;
    dlc = strList.count() > 8 ? 8 : strList.count();
    for(int i = 0;i < dlc;i ++)
        data[i] = strList.at(i).toInt(Q_NULLPTR,16);

    if(canthread->sendData(QVariant(ui->comboBox_4->currentIndex()).toUInt(),
                           QVariant(ui->sendIDEdit->text().toInt(Q_NULLPTR,16)).toUInt(),
                           ui->comboBox_5->currentIndex(),
                           ui->comboBox_6->currentIndex(),
                           data,dlc))
    {//发送成功，打印数据
        QStringList messageList;

        messageList.clear();
        messageList << QTime::currentTime().toString("hh:mm:ss zzz");//时间
        messageList << "无";//时间
        messageList << "无";//时间
        messageList << "CH" + QString::number(QVariant(ui->comboBox_4->currentIndex()).toUInt());
        messageList << "发送";//收发
        messageList << "0x" + ui->sendIDEdit->text().toUpper();//ID
        messageList << ((ui->comboBox_5->currentIndex() == 0) ? "数据帧" : "远程帧");//类型
        messageList << ((ui->comboBox_6->currentIndex() == 0) ? "标准帧" : "扩展帧");//Frame
        QString str = "";
        if(ui->comboBox_5->currentIndex() == 0)//数据帧显示数据
        {
            messageList << "0x" + QString::number(dlc,16).toUpper();//长度
            str = "x| ";
            for(int j = 0;j < dlc;j ++)
                str += QString("%1 ").arg((unsigned char)data[j],2,16,QChar('0')).toUpper();//QString::number((unsigned char)data[j],16) + " ";
        }
        else
            messageList << "0x0";//长度
        messageList << str;//数据
        AddDataToList(messageList);
    }
    else
        QMessageBox::warning(this,"警告","数据发送失败！");
}

void MainWindow::on_DC_Close_btn_clicked()
{
    ui->sendDataEdit->setText("00 10 3A 00 00 00 00 00");
    QStringList strList = ui->sendDataEdit->text().split(" ");
    unsigned char data[8];
    memset(data,0,8);
    UINT dlc = 0;
    dlc = strList.count() > 8 ? 8 : strList.count();
    for(int i = 0;i < dlc;i ++)
        data[i] = strList.at(i).toInt(Q_NULLPTR,16);

    if(canthread->sendData(QVariant(ui->comboBox_4->currentIndex()).toUInt(),
                           QVariant(ui->sendIDEdit->text().toInt(Q_NULLPTR,16)).toUInt(),
                           ui->comboBox_5->currentIndex(),
                           ui->comboBox_6->currentIndex(),
                           data,dlc))
    {//发送成功，打印数据
        QStringList messageList;

        messageList.clear();
        messageList << QTime::currentTime().toString("hh:mm:ss zzz");//时间
        messageList << "无";//时间
        messageList << "无";//时间
        messageList << "CH" + QString::number(QVariant(ui->comboBox_4->currentIndex()).toUInt());
        messageList << "发送";//收发
        messageList << "0x" + ui->sendIDEdit->text().toUpper();//ID
        messageList << ((ui->comboBox_5->currentIndex() == 0) ? "数据帧" : "远程帧");//类型
        messageList << ((ui->comboBox_6->currentIndex() == 0) ? "标准帧" : "扩展帧");//Frame
        QString str = "";
        if(ui->comboBox_5->currentIndex() == 0)//数据帧显示数据
        {
            messageList << "0x" + QString::number(dlc,16).toUpper();//长度
            str = "x| ";
            for(int j = 0;j < dlc;j ++)
                str += QString("%1 ").arg((unsigned char)data[j],2,16,QChar('0')).toUpper();//QString::number((unsigned char)data[j],16) + " ";
        }
        else
            messageList << "0x0";//长度
        messageList << str;//数据
        AddDataToList(messageList);
    }
    else
        QMessageBox::warning(this,"警告","数据发送失败！");
}



void MainWindow::on_Zbalance_open_btn_clicked()
{
    ui->sendDataEdit->setText("00 10 37 03 00 F0 00 00");
    QStringList strList = ui->sendDataEdit->text().split(" ");
    unsigned char data[8];
    memset(data,0,8);
    UINT dlc = 0;
    dlc = strList.count() > 8 ? 8 : strList.count();
    for(int i = 0;i < dlc;i ++)
        data[i] = strList.at(i).toInt(Q_NULLPTR,16);

    if(canthread->sendData(QVariant(ui->comboBox_4->currentIndex()).toUInt(),
                           QVariant(ui->sendIDEdit->text().toInt(Q_NULLPTR,16)).toUInt(),
                           ui->comboBox_5->currentIndex(),
                           ui->comboBox_6->currentIndex(),
                           data,dlc))
    {//发送成功，打印数据
        QStringList messageList;

        messageList.clear();
        messageList << QTime::currentTime().toString("hh:mm:ss zzz");//时间
        messageList << "无";//时间
        messageList << "无";//时间
        messageList << "CH" + QString::number(QVariant(ui->comboBox_4->currentIndex()).toUInt());
        messageList << "发送";//收发
        messageList << "0x" + ui->sendIDEdit->text().toUpper();//ID
        messageList << ((ui->comboBox_5->currentIndex() == 0) ? "数据帧" : "远程帧");//类型
        messageList << ((ui->comboBox_6->currentIndex() == 0) ? "标准帧" : "扩展帧");//Frame
        QString str = "";
        if(ui->comboBox_5->currentIndex() == 0)//数据帧显示数据
        {
            messageList << "0x" + QString::number(dlc,16).toUpper();//长度
            str = "x| ";
            for(int j = 0;j < dlc;j ++)
                str += QString("%1 ").arg((unsigned char)data[j],2,16,QChar('0')).toUpper();//QString::number((unsigned char)data[j],16) + " ";
        }
        else
            messageList << "0x0";//长度
        messageList << str;//数据
        AddDataToList(messageList);
    }
    else
        QMessageBox::warning(this,"警告","数据发送失败！");
}

void MainWindow::on_balance_close_btn_clicked()
{
    ui->sendDataEdit->setText("00 10 37 02 00 00 00 00");
    QStringList strList = ui->sendDataEdit->text().split(" ");
    unsigned char data[8];
    memset(data,0,8);
    UINT dlc = 0;
    dlc = strList.count() > 8 ? 8 : strList.count();
    for(int i = 0;i < dlc;i ++)
        data[i] = strList.at(i).toInt(Q_NULLPTR,16);

    if(canthread->sendData(QVariant(ui->comboBox_4->currentIndex()).toUInt(),
                           QVariant(ui->sendIDEdit->text().toInt(Q_NULLPTR,16)).toUInt(),
                           ui->comboBox_5->currentIndex(),
                           ui->comboBox_6->currentIndex(),
                           data,dlc))
    {//发送成功，打印数据
        QStringList messageList;

        messageList.clear();
        messageList << QTime::currentTime().toString("hh:mm:ss zzz");//时间
        messageList << "无";//时间
        messageList << "无";//时间
        messageList << "CH" + QString::number(QVariant(ui->comboBox_4->currentIndex()).toUInt());
        messageList << "发送";//收发
        messageList << "0x" + ui->sendIDEdit->text().toUpper();//ID
        messageList << ((ui->comboBox_5->currentIndex() == 0) ? "数据帧" : "远程帧");//类型
        messageList << ((ui->comboBox_6->currentIndex() == 0) ? "标准帧" : "扩展帧");//Frame
        QString str = "";
        if(ui->comboBox_5->currentIndex() == 0)//数据帧显示数据
        {
            messageList << "0x" + QString::number(dlc,16).toUpper();//长度
            str = "x| ";
            for(int j = 0;j < dlc;j ++)
                str += QString("%1 ").arg((unsigned char)data[j],2,16,QChar('0')).toUpper();//QString::number((unsigned char)data[j],16) + " ";
        }
        else
            messageList << "0x0";//长度
        messageList << str;//数据
        AddDataToList(messageList);
    }
    else
        QMessageBox::warning(this,"警告","数据发送失败！");
}

void MainWindow::on_Sbalance_open_btn_clicked()
{
    QString c;
    QString a= "00 10 37 01 04";
    if(ui->balance_time->text().size()>0)
    {
        int b=ui->balance_time->text().toInt(Q_NULLPTR,10);
        QString d=QString("%1").arg(b,4,16,QLatin1Char('0'));

        int n = d.length();
        while(n-2 > 0)
        {
            n = n - 2;
            d.insert(n," ");
        }

        c = QString("%1 %2 00").arg(a).arg(d);
//        qDebug()<<ui->balance_time->text().toInt(Q_NULLPTR,16);

//        qDebug()<<c;

    }

    ui->sendDataEdit->setText(c);
    QStringList strList = ui->sendDataEdit->text().split(" ");
    unsigned char data[8];
    memset(data,0,8);
    UINT dlc = 0;
    dlc = strList.count() > 8 ? 8 : strList.count();
    for(int i = 0;i < dlc;i ++)
        data[i] = strList.at(i).toInt(Q_NULLPTR,16);

    if(canthread->sendData(QVariant(ui->comboBox_4->currentIndex()).toUInt(),
                           QVariant(ui->sendIDEdit->text().toInt(Q_NULLPTR,16)).toUInt(),
                           ui->comboBox_5->currentIndex(),
                           ui->comboBox_6->currentIndex(),
                           data,dlc))
    {//发送成功，打印数据
        QStringList messageList;

        messageList.clear();
        messageList << QTime::currentTime().toString("hh:mm:ss zzz");//时间
        messageList << "无";//时间
        messageList << "无";//时间
        messageList << "CH" + QString::number(QVariant(ui->comboBox_4->currentIndex()).toUInt());
        messageList << "发送";//收发
        messageList << "0x" + ui->sendIDEdit->text().toUpper();//ID
        messageList << ((ui->comboBox_5->currentIndex() == 0) ? "数据帧" : "远程帧");//类型
        messageList << ((ui->comboBox_6->currentIndex() == 0) ? "标准帧" : "扩展帧");//Frame
        QString str = "";
        if(ui->comboBox_5->currentIndex() == 0)//数据帧显示数据
        {
            messageList << "0x" + QString::number(dlc,16).toUpper();//长度
            str = "x| ";
            for(int j = 0;j < dlc;j ++)
                str += QString("%1 ").arg((unsigned char)data[j],2,16,QChar('0')).toUpper();//QString::number((unsigned char)data[j],16) + " ";
        }
        else
            messageList << "0x0";//长度
        messageList << str;//数据
        AddDataToList(messageList);
    }
    else
        QMessageBox::warning(this,"警告","数据发送失败！");
}


