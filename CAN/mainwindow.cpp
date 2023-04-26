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

    QValidator*validator=new QIntValidator(1,9,this);
    ui->BBM_edit->setValidator(validator);

    //lineedit文本框输入类型限制 int型
//   QValidator * validator = new QIntValidator(0,999,this);
//   V_calibration_edit->setValidator(validator);
   /*也可以表示成
   QLineEdit * edit new QLineEdit(this);
   edit->setValidator(new QIntValidator(0,999,this));
   */

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

        QString m=messageList.at(9);//接收的数据
        qDebug()<<"接收信号"<<messageList;
        //数据实时监测
        //ID号识别
        qDebug()<<"接收的数据整个id"<<messageList.at(5);
        QString id= messageList.at(5);
        int ID =id.mid(8,2).toInt(Q_NULLPTR,10);
//        int ID =id.mid(7,3).toInt(Q_NULLPTR,10);
        qDebug()<<"id号"<<id.mid(8,2);

//        qDebug()<<"id号"<<id.mid(7,3);
        //id号
        if(ID==10)
        {
            qDebug()<<"四路电压"<<messageList.at(9);

            QString v4_4=messageList.at(9);
            QString v1_4=v4_4.mid(3).remove(QRegExp("\\s"));//0F000F000F000F00
            qDebug()<<"四路电压"<<v1_4;
            //删除t所有空格
            QString v1= v1_4.mid(2,2)+v1_4.mid(0,2);
            QString v2= v1_4.mid(6,2)+v1_4.mid(4,2);
            QString v3= v1_4.mid(10,2)+v1_4.mid(8,2);
            QString v4= v1_4.mid(14,2)+v1_4.mid(12,2);

            int v11=v1.toInt(Q_NULLPTR,16);
            int v22=v2.toInt(Q_NULLPTR,16);
            int v33=v3.toInt(Q_NULLPTR,16);
            int v44=v4.toInt(Q_NULLPTR,16);

            QString V1=QString::number(v11,10);
            QString V2=QString::number(v22,10);
            QString V3=QString::number(v33,10);
            QString V4=QString::number(v44,10);

            qDebug()<<"v1"<<V1;
            qDebug()<<"v2"<<V2;
            qDebug()<<"v3"<<V3;
            qDebug()<<"v4"<<V4;

            ui->V1_edit->setText(V1);
            ui->V2_edit->setText(V2);
            ui->V3_edit->setText(V3);
            ui->V4_edit->setText(V4);

            return ;
        }
        if(ID==11)
        {
            qDebug()<<"八个温度采样"<<messageList.at(9);
            QString t=messageList.at(9);
            QString T=t.mid(3).remove(QRegExp("\\s"));
            qDebug()<<"T"<<T; //7F7F7F7F7F7F7F7F
            int t1=T.mid(0,2).toInt(Q_NULLPTR,16);
            int t2= T.mid(2,2).toInt(Q_NULLPTR,16);
            int t3= T.mid(4,2).toInt(Q_NULLPTR,16);
            int t4= T.mid(6,2).toInt(Q_NULLPTR,16);
            int t5= T.mid(8,2).toInt(Q_NULLPTR,16);
            int t6= T.mid(10,2).toInt(Q_NULLPTR,16);
            int t7= T.mid(12,2).toInt(Q_NULLPTR,16);
            int t8= T.mid(14,2).toInt(Q_NULLPTR,16);
            qDebug()<<"t1"<<t1;
            qDebug()<<"t2"<<t2;

            ui->t1_edit->setText(QString::number(t1,10));

            ui->t2_edit->setText(QString::number(t2,10));

            ui->t3_edit->setText(QString::number(t3,10));

            ui->t4_edit->setText(QString::number(t4,10));

            ui->t5_edit->setText(QString::number(t5,10));

            ui->t6_edit->setText(QString::number(t6,10));

            ui->t7_edit->setText(QString::number(t7,10));

            ui->t8_edit->setText(QString::number(t8,10));


            return ;
        }
        if(ID==12)
        {
            qDebug()<<"四个温度+电流采样"<<messageList.at(9);
            QString t=messageList.at(9);
            QString T=t.mid(3).remove(QRegExp("\\s"));
            qDebug()<<"T"<<T; //7F 7F 7F 7F 01 0C 00 00
            int t9=T.mid(0,2).toInt(Q_NULLPTR,16);
            int t10= T.mid(2,2).toInt(Q_NULLPTR,16);
            int t11= T.mid(4,2).toInt(Q_NULLPTR,16);
            int t12= T.mid(6,2).toInt(Q_NULLPTR,16);
            int I= T.mid(8,4).toInt(Q_NULLPTR,16);//电流

            int i=65535-I;


            ui->I_edit->setText(QString::number(i,10));

            ui->t9_edit->setText(QString::number(t9,10));

            ui->t10_edit->setText(QString::number(t10,10));

            ui->t11_edit->setText(QString::number(t11,10));

            ui->t12_edit->setText(QString::number(t12,10));




            return ;
        }
        if(ID==13)
        {
            qDebug()<<"平衡信息"<<messageList.at(9);
            QString t=messageList.at(9);
            QString T=t.mid(3).remove(QRegExp("\\s"));

            qDebug()<<"T"<<T; //02状态 00通道 00 C8压差 00 F0周期 01交流信号 00 SOC

            int state = T.mid(0,2).toInt(Q_NULLPTR,16);
//            int channel= T.mid(2,2).toInt(Q_NULLPTR,16);
            int VD= T.mid(4,4).toInt(Q_NULLPTR,16);
            int tim= T.mid(8,4).toInt(Q_NULLPTR,16);
            int signal= T.mid(12,2).toInt(Q_NULLPTR,16);
            int SOC= T.mid(14,2).toInt(Q_NULLPTR,16);

            qDebug()<<"state"<<state;
            qDebug()<<"VD"<<VD;
            qDebug()<<"tim"<<tim;
            qDebug()<<"signal"<<signal;
            qDebug()<<"SOC"<<SOC;

            ui->balance_v_edit->setText(QString::number(VD,10));
            ui->balance_time_edit->setText(QString::number(tim,10));
            ui->soc_edit->setText(QString::number(SOC,10));
            if (state==1)
            {
                ui->balance_auto->setChecked(0);
                ui->balance_manual->setChecked(1);
                ui->balance_close->setChecked(0);

            }
            if (state==0)
            {
                ui->balance_manual->setChecked(0);
                 ui->balance_auto->setChecked(1);
                ui->balance_close->setChecked(0);

            }
            if (state==2)
            {
                ui->balance_auto->setChecked(0);
                ui->balance_manual->setChecked(0);
                ui->balance_close->setChecked(1);

            }

            if (signal==0)
            {
                ui->signal_con_radio->setChecked(1);
                ui->signal_discon_radio->setChecked(0);

            }
            if (signal==1)
            {
                ui->signal_discon_radio->setChecked(1);
                ui->signal_con_radio->setChecked(0);

            }

            return ;
        }


        /*版本号判断*/
        if(m.mid(3).startsWith("00 10 3F"))

        {

           QString version =m.mid(12).trimmed();//过滤空白字符
           version.mid(0,2).append("年");
           version.mid(3,5).append("月");
           version.mid(7,9).append("日");
           version.mid(10,12).append(".");
           qDebug()<<version;
           //字符串截取
           /*
            mid(int position, int n = -1) const

            两个参数，第一个是起始位置，第二个是取串的长度。如果省略第二个参数，则会从起始位置截取到末尾
            */
           QString Version=version.mid(0,2).append("年")+version.mid(3,2).append("月")
                   +version.mid(6,2).append("日")+version.mid(9,2).append(".")
                   +version.mid(12,2);
           qDebug()<<"程序版本"<<Version;
           ui->read_version_edit->setText(Version);
           qDebug()<<version.mid(0,2).append("年");
           qDebug()<<version.mid(3,2).append("月");
           qDebug()<<version.mid(6,2).append("日");
           qDebug()<<version.mid(9,2).append(".");
           qDebug()<<version.mid(12,2);


//              qDebug()<<m.mid(12);
//                QString str= messageList.mid(7).join("");
//                ui->read_version_edit->setText(str);
        }

        return ;

//        if(m.mid(3).startsWith("00 10 3F"))

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

    if(canthread->sendData(QVariant(ui->comboBox_4->currentIndex()).toInt(),
                           QVariant(ui->sendIDEdit->text().toInt(Q_NULLPTR,16)).toInt(),
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

void MainWindow::                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   on_Sbalance_open_btn_clicked()
{
    QString c;
    QString a= "00 10 37 01 ";
    QString channel;
    int d = ui->comboBox_7->currentIndex();
    qDebug()<<"currentIndex--int"<<d;
    switch(d)
    {
    case 0:
        channel="04";
        break;
    case 1:
        channel="01";
        break;
    case 2:
        channel="02";
        break;
    case 3:
        channel="03";
        break;
    default:
            break;
    }
//    channel= QString::number(d);

    qDebug()<<"channel"<<channel;

    a+=channel;
    qDebug()<<"a"<<a;

//    ui->comboBox_7->addItem(channel);

    int time =ui->balance_time->text().toInt(Q_NULLPTR,10);


    QString Q=QString("%1").arg(time,4,16,QLatin1Char('0'));
    int n = Q.length();
    while(n-2 > 0)
    {
        n = n - 2;
        Q.insert(n," ");
    }
    a+=" ";
    a+=Q;
    qDebug()<<"a"<<a;
    c = QString("%1 00").arg(a);


    qDebug()<<ui->balance_time->text().toInt(Q_NULLPTR,16);

    qDebug()<<c;


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





void MainWindow::on_liquid_open_btn_clicked()
{
    ui->sendDataEdit->setText("00 10 38 01 00 00 00 00");
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

void MainWindow::on_liquid_close_btn_clicked()
{
    ui->sendDataEdit->setText("00 10 38 00 00 00 00 00");
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

void MainWindow::on_V_calibration_btn_clicked()
{
    if(ui->V_calibration_edit->text().size()>0)
    {

        int b=ui->V_calibration_edit->text().toInt(Q_NULLPTR,10);
        QString d=QString("%1").arg(b,4,16,QLatin1Char('0'));

        qDebug()<<"校准电压"<<d<<endl;
        QString start =d.mid(2,2);
        start+=d.mid(0,2);
        int n = d.length();
        while(n-2 > 0)
        {
            n = n - 2;
            start.insert(n," ");
        }


        qDebug()<<"校准电压"<<d<<endl;


        QString c = QString("00 10 34 %1 00 00 00").arg(start);
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




}

void MainWindow::on_I_calibration_btn_clicked()
{
    if(ui->I_calibration_edit->text().size()>0)
    {

        int b=ui->I_calibration_edit->text().toInt(Q_NULLPTR,10);
        QString d=QString("%1").arg(b,4,16,QLatin1Char('0'));

        qDebug()<<"校准电流"<<d<<endl;
        QString start =d.mid(2,2);
        start+=d.mid(0,2);
        int n = d.length();
        while(n-2 > 0)
        {
            n = n - 2;
            start.insert(n," ");
        }

        QString c = QString("00 10 3B %1 00 00 00").arg(start);
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
}

void MainWindow::on_read_version_btn_clicked()
{
    ui->sendDataEdit->setText("00 10 3F 00 00 00 00 00");
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

void MainWindow::on_BBM_edit_btn_clicked()
{

    if(ui->BBM_edit->text().size()>0)
    {

        QString BBM_number="00 10 40 ";
        BBM_number=BBM_number.append(ui->BBM_edit->text());
        BBM_number=BBM_number.append(" 00 00 00 00");

        qDebug()<<"BBM_number"<<BBM_number;


    ui->sendDataEdit->setText(BBM_number);
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
}

void MainWindow::on_BMM_Restart_clicked()
{

    ui->sendDataEdit->setText("00 10 39 00 00 00 00 00");
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
