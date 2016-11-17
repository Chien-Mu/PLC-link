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

void PLC::serial_write(QByteArray value){
    requestData = value;
    cond.wakeOne();
}

void PLC::stop(){
    quit = true;
    cond.wakeOne();
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

    //write and read
    while(!quit){
        mutex.lock();
        cond.wait(&mutex);

        //加這段是程式鎖在上面(待機中)時，此時按stop扭才會處理
        if(quit){
            mutex.unlock();
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
            if (serial.waitForReadyRead(5000)) { //若一直沒接收到，會在這等待5秒(此時按stop扭雖然不會馬上停止，但quit 還是會被設成false，之後自動跳到後面結束)
                QByteArray responseData = serial.readAll(); //接收到後抓近來
                while (serial.waitForReadyRead(10)) //看看後面有沒有殘餘資料
                    responseData += serial.readAll();
                emit status(">..Received :" + responseData.toHex());
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

        mutex.unlock();
    }

    if (serial.isOpen())
        serial.close();
    emit status("0");
}


