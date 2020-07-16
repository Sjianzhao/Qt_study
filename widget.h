//#ifndef WIDGET_H
//#define WIDGET_H

//#include <QWidget>

//namespace Ui {
//class Widget;
//}

//class Widget : public QWidget
//{
//    Q_OBJECT

//public:
//    explicit Widget(QWidget *parent = 0);
//    ~Widget();

//private:
//    Ui::Widget *ui;
//};

//#endif // WIDGET_H


#ifndef WIDGET_H
#define WIDGET_H


#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    void SerialPort_DealMessage(QByteArray Message_Receive);
    ~Widget();

private slots:
    void handleTimeout();
    void Uart_Message_Deal();
    void on_Uart_Operate_Button_clicked();
    void SerialPort_ReadyRead();
    void on_Uart_Message_Send_Button_clicked();
    void on_Uart_Message_Send_Clear_Button_clicked();
    void on_Uart_Message_Receive_Clear_Button_clicked();


    void on_Uart_Message_Send_HEX_CheckBox_stateChanged(int arg1);

    void on_Uart_Message_Receive_HEX_CheckBox_stateChanged(int arg1);

private:
    Ui::Widget *ui;
    QSerialPort Serial;         //串口类
    QTimer *pMy_Timer;          //定时器
    QTimer *pMy_Timer_Uart;     //定时器
};

#endif // MAINWIDGET_H








