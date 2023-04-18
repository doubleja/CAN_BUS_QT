#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "canthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void AddDataToList(QStringList strList);
private slots:
    void onGetProtocolData(VCI_CAN_OBJ *vci,unsigned int dwRel,unsigned int channel);
    void onBoardInfo(VCI_BOARD_INFO vbi);
    void on_pushButton_clicked();

    void on_sendBtn_clicked();

    void on_cleanListBtn_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_DC_Open_btn_clicked();

    void on_DC_Close_btn_clicked();


    void on_Zbalance_open_btn_clicked();

    void on_balance_close_btn_clicked();

    void on_Sbalance_open_btn_clicked();

    void on_pushButton_13_clicked();

private:
    Ui::MainWindow *ui;
    CANThread *canthread;
};

#endif // MAINWINDOW_H
