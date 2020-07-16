#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QThread>

unsigned int USART_RX_STA = 0;                      //清零
//串口接收缓存区
unsigned char USART_RX_BUF[1000];                   //接收缓冲

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    Widget::setWindowIcon(QIcon("../uart/xph001.ico"));
    setWindowTitle(tr("串口调试"));         //设置标题

    //定时器建立
    pMy_Timer = new QTimer(this);
    pMy_Timer->stop();                    //关闭定时器扫描

    pMy_Timer_Uart = new QTimer(this);
    pMy_Timer_Uart->start(200);           //开启定时器扫描

    connect(pMy_Timer,&QTimer::timeout,this,&Widget::handleTimeout);
    connect(pMy_Timer_Uart,&QTimer::timeout,this,&Widget::Uart_Message_Deal);
    //连接信号和槽
    QObject::connect(&Serial, &QSerialPort::readyRead, this, &Widget::SerialPort_ReadyRead);


    //通过QSerialPortInfo查找可用串口
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->Uart_PortName_Box->addItem(info.portName());
    }
    ui->Uart_Message_Send_Button->setEnabled(false);    //发送按钮关闭
}

Widget::~Widget()
{
    delete ui;
}

//串口开关控制
void Widget::on_Uart_Operate_Button_clicked()
{
    static int Uart_State;      //串口状态（0：打开 1：关闭）
    switch(Uart_State)
    {
        //打开串口
        case 0:
        {
            //设置串口名
            Serial.setPortName(ui->Uart_PortName_Box->currentText());
            //设置波特率
            Serial.setBaudRate(ui->Uart_Baudrate_Box->currentText().toInt());
            //设置数据位数
            switch(ui->Uart_DateBit_Box->currentIndex())
            {
                case 8:
                {
                    Serial.setDataBits(QSerialPort::Data8);
                    break;
                }
                case 7:
                {
                    Serial.setDataBits(QSerialPort::Data7);
                    break;
                }
                case 6:
                {
                    Serial.setDataBits(QSerialPort::Data6);
                    break;
                }
                case 5:
                {
                    Serial.setDataBits(QSerialPort::Data5);
                    break;
                }
                default: break;
            }
            //设置奇偶校验
            switch(ui->Uart_Parity_Box->currentIndex())
            {
                //无校验
                case 0:
                {
                    Serial.setParity(QSerialPort::NoParity);
                    break;
                }
                //奇校验
                case 1:
                {
                    Serial.setParity(QSerialPort::OddParity);
                    break;
                }
                //偶校验
                case 2:
                {
                    Serial.setParity(QSerialPort::EvenParity);
                    break;
                }
                default:
                    break;
            }

            //设置停止位
            switch(ui->Uart_StopBit_Box->currentIndex())
            {
                case 1:
                {
                    Serial.setStopBits(QSerialPort::OneStop);
                    break;
                }
                case 2:
                {
                    Serial.setStopBits(QSerialPort::TwoStop);
                    break;
                }
                default:
                    break;
            }
            //设置流控制
            Serial.setFlowControl(QSerialPort::NoFlowControl);

            //打开串口
            if(!Serial.open(QIODevice::ReadWrite))
            {
                QMessageBox::about(this, "提示", "无法打开串口！");
                return;
            }

            ui->Uart_PortName_Box->setEnabled(false);        //端口号
            ui->Uart_Parity_Box->setEnabled(false);          //奇偶校验
            ui->Uart_DateBit_Box->setEnabled(false);         //数据位
            ui->Uart_StopBit_Box->setEnabled(false);         //停止位
            ui->Uart_Baudrate_Box->setEnabled(false);        //波特率
            ui->Uart_Message_Send_Button->setEnabled(true);    //发送按钮打开
            ui->Uart_Operate_Button->setText(tr("关闭串口"));
            Uart_State = 1;           //标记串口已打开
            break;
        }

        //关闭串口
        case 1:
        {
            ui->Uart_PortName_Box->setEnabled(true);        //端口号
            ui->Uart_Parity_Box->setEnabled(true);          //奇偶校验
            ui->Uart_DateBit_Box->setEnabled(true);         //数据位
            ui->Uart_StopBit_Box->setEnabled(true);         //停止位
            ui->Uart_Baudrate_Box->setEnabled(true);        //波特率
            ui->Uart_Message_Send_Button->setEnabled(false);    //发送按钮关闭
            ui->Uart_Operate_Button->setText(tr("打开串口"));
            Serial.close();           //关闭串口
            Uart_State = 0;           //标记串口已关闭
            break;
        }
    }
}

//接收数据
void Widget::SerialPort_ReadyRead()
{
    //从接收缓冲区中读取数据
    QByteArray Serial_Receive_Buffer = Serial.readAll();                       //读取全部缓冲区数据
    SerialPort_DealMessage(Serial_Receive_Buffer);           //数据处理回调函数
    QString Receive;
    //十进制接收
    if(ui->Uart_Message_Receive_HEX_CheckBox->isChecked()==false)
    {
        //从界面中读取以前收到的数据
        Receive = ui->Uart_Message_Receive_TextBrowser->toPlainText();
        //加上接收到的数据
        Receive += QString(Serial_Receive_Buffer);
        //清空以前的显示
        ui->Uart_Message_Receive_TextBrowser->clear();
        //重新显示(字符串)
        ui->Uart_Message_Receive_TextBrowser->append(Receive);
    }
    //十六进制接收
    else {
        //从界面中读取以前收到的数据
        Receive = QByteArray::fromHex(ui->Uart_Message_Receive_TextBrowser->toPlainText().toUtf8()).data();
        //加上接收到的数据
        Receive += QString(Serial_Receive_Buffer);
        //清空以前的显示
        ui->Uart_Message_Receive_TextBrowser->clear();
        //重新显示(十六进制)
        ui->Uart_Message_Receive_TextBrowser->append(Receive.toUtf8().toHex().toUpper().data());
    }
}

//发送数据
void Widget::on_Uart_Message_Send_Button_clicked()
{
    QByteArray Send_Buffer;     //发送缓冲区
    //字符发送
    if(ui->Uart_Message_Send_HEX_CheckBox->isChecked()==false)
    {
        Send_Buffer = ui->Uart_Message_Send_TextEdit->toPlainText().toUtf8();
        if(ui->Uart_Message_Send_LF_CheckBox->isChecked()==true)
        {
            Send_Buffer+="\r\n";        //如果换行选项被勾中默认添加回车选项
        }
        Serial.write(Send_Buffer);
        ui->Uart_Message_Receive_TextBrowser->setText(Send_Buffer);
    }
    //十六进制发送
    else
    {
        Send_Buffer=QByteArray::fromHex(ui->Uart_Message_Send_TextEdit->toPlainText().toUtf8()).data();
        Send_Buffer+="\r\n";
        if(ui->Uart_Message_Send_LF_CheckBox->isChecked()==true)
        {
            Send_Buffer+="\r\n";        //如果换行选项被勾中默认添加回车选项
        }
        Serial.write(Send_Buffer);
        ui->Uart_Message_Receive_TextBrowser->setText(ui->Uart_Message_Send_TextEdit->toPlainText().toUtf8().toHex().toUpper().data());
    }
}

//清除发送数据
void Widget::on_Uart_Message_Send_Clear_Button_clicked()
{
    ui->Uart_Message_Send_TextEdit->clear();
}

//清除接收数据
void Widget::on_Uart_Message_Receive_Clear_Button_clicked()
{
    ui->Uart_Message_Receive_TextBrowser->clear();
}

//发送文本区十六进制与字符串转换
void Widget::on_Uart_Message_Send_HEX_CheckBox_stateChanged(int arg1)
{
    //16进制显示
    if(arg1!=0)     //复选框被勾中
    {
        ui->Uart_Message_Send_TextEdit->
                setText(ui->Uart_Message_Send_TextEdit->toPlainText().toUtf8().
                        toHex().toUpper().data());
    }
    //字符串显示     复选框被取消
    else {
        ui->Uart_Message_Send_TextEdit->
                setText(QByteArray::fromHex(ui->Uart_Message_Send_TextEdit->toPlainText().toUtf8()).
                        data());
    }
}

//显示文本区十六进制与字符串转换
void Widget::on_Uart_Message_Receive_HEX_CheckBox_stateChanged(int arg1)
{
    //16进制显示
    if(arg1!=0)     //复选框被钩中
    {
        ui->Uart_Message_Receive_TextBrowser->
                setText(ui->Uart_Message_Receive_TextBrowser->toPlainText().toUtf8().
                        toHex().toUpper().data());
    }
    //字符串显示     复选框被取消
    else {
        ui->Uart_Message_Receive_TextBrowser->
                setText(QByteArray::fromHex(ui->Uart_Message_Receive_TextBrowser->toPlainText().toUtf8()).
                        data());
    }
}

void Widget::handleTimeout()
{
    USART_RX_STA|=1<<15;                           //标记接收完成
    pMy_Timer->stop();                             //关闭定时器
}

//串口数据处理
void Widget::SerialPort_DealMessage(QByteArray Message_Receive)
{
    qint16 Temp_CLC = 0;
    unsigned char Recive_Date_Temp;      //临时接收值
    for(Temp_CLC = 0;Temp_CLC <Message_Receive.length();Temp_CLC++)
    {
        Recive_Date_Temp =Message_Receive[Temp_CLC];
        if((USART_RX_STA&(1<<15))==0)                   //接收完的一批数据,还没有被处理,则不再接收其他数据
         {
            if(USART_RX_STA<1000)       //还可以接收数据
            {
                pMy_Timer->setInterval(10);             //定时器从新更新时间
                if(USART_RX_STA==0)                     //使能定时器的中断
                {
                    pMy_Timer->start(10);               //使能定时器
                }
                USART_RX_BUF[USART_RX_STA++]=Recive_Date_Temp;       //记录接收到的值
            }
            else
            {
                USART_RX_STA|=1<<15;                   //强制标记接收完成
            }
        }
    }
}


//串口数据处理
void Widget::Uart_Message_Deal()
{
    qint16 Receive_Buffer_Length;
    qint16 Temp_CLC;
    if(USART_RX_STA&0X8000)		//接收到一次数据了
    {
          Receive_Buffer_Length = USART_RX_STA&0X7FFF - 1;
          for(Temp_CLC=0;Temp_CLC<=Receive_Buffer_Length;Temp_CLC++)
          {
              qDebug()<<USART_RX_BUF[Temp_CLC];
          }
          USART_RX_STA = 0;
    }
}


