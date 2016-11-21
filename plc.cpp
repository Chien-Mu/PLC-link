#include "plc.h"
#include <QtSerialPort/QSerialPort>

PLC::PLC(){
    this->quit = false;
}

PLC::~PLC(){
    this->quit = true;
}

void PLC::setCOM(QString COM_ID, int DelayTime, int Timeout){
    this->COM_ID = COM_ID;
    this->DelayTime = DelayTime;
    this->Timeout = Timeout;
}

void PLC::cmd(PLC_CMD PlcCommand){
    mutex.lock();
    //cond.wait(&mutex);

    this->PlcCommand = PlcCommand;

    mutex.unlock();
}

void PLC::stop(){
    quit = true;
}

void PLC::run(){
    quit = false; //這裡要初始化，不然第二次執行時，他會殘留上次執行的值

    QSerialPort serial;
    //set
    serial.setPortName(COM_ID);
    serial.setBaudRate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data7); //非預設
    serial.setParity(QSerialPort::EvenParity); //非預設
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    //open
    if (serial.open(QIODevice::ReadWrite)) {
        emit status("1");
    } else {
        emit status(serial.errorString());
        return;
    }

    QByteArray requestData;
    int bytesWritten;
    PlcCommand = Read_M; //default
    PLC_CMD cmding = Read_M;
    QByteArray station = "00";
    QByteArray PC = "FF";

    //write and read
    while(!quit){        

        //cond.wakeAll(); //解鎖
        cmding = PlcCommand; //避免 PlcCommand 被更改時，引起判斷上的錯誤
        PlcCommand = Read_M; //default

        //encode
        switch (cmding)
        {
            case M100_ON:
                requestData = QString(5).toLocal8Bit() + station + PC + "BW5M01000115A";
                break;
            case M100_OFF:
                requestData = QString(5).toLocal8Bit() + station + PC + "BW5M010001059";
                break;
            default: //0 Read_M
                requestData = QString(5).toLocal8Bit() + station + PC + "BRAM01000433"; //read M100~M103
                break;
        }


        //add buffer
        bytesWritten = serial.write(requestData);

        //檢查buffer 是否有加入不完全的錯誤
        if (bytesWritten == -1)
            emit status(QString("Failed to write the data to port %1, error: %2").
                    arg(serial.portName()).arg(serial.errorString()));
        else if (bytesWritten != requestData.size())
            emit status(QString("Failed to write all the data to port %1, error: %2").
                    arg(serial.portName()).arg(serial.errorString()));

        //AP COM push buffer 出去
        if(serial.waitForBytesWritten(5000)){

            // read response 抓回傳資料
            if (serial.waitForReadyRead(50000)) { //若一直沒接收到，會在這等待5秒(此時按stop扭雖然不會馬上停止，但quit 還是會被設成false，之後自動跳到後面結束)
                QByteArray responseData = serial.readAll(); //接收到後抓近來
                while (serial.waitForReadyRead(10)) //看看後面有沒有殘餘資料
                    responseData += serial.readAll();
                emit status(">" + responseData + "\n" + responseData.toHex());

                //回答了解
                if(cmding == Read_M){
                    msleep(50);
                    serial.write(QString(6).toLocal8Bit() + station + PC); // 06 00 FF
                    if(!serial.waitForBytesWritten(5000))
                        emit status(QString("waitForBytesWritten() timed out for port %1, error: %2").
                            arg(serial.portName()).arg(serial.errorString()));
                }

            }else{
                emit status(tr("Wait read response timeout"));                                    
            }

            //撿查接收後 是否有錯誤
            if (serial.error() == QSerialPort::ReadError)
                emit status(QString("Failed to read from port %1, error: %2").
                            arg(serial.portName()).arg(serial.errorString()));

        }else{
            emit status(QString("waitForBytesWritten() timed out for port %1, error: %2").
                    arg(serial.portName()).arg(serial.errorString()));
        }

        msleep(100);

    }

    if (serial.isOpen())
        serial.close();
    emit status("0");
}


