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

void PLC::process(QByteArray value){
    if(value.length() >= 6){
        if(value.at(5) == '0')
            emit M100(false);
        else if(value.at(5) == '1')
            emit M100(true);
    }
    emit status(">\n exe:" + value + " - " + value->toHex());
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

    PlcCommand = Read_M; //default
    PLC_CMD cmding = Read_M;
    QByteArray station = "00";
    QByteArray PC = "FF";
    QByteArray STX = QString(2).toLocal8Bit();
    QByteArray ETX = QString(3).toLocal8Bit();
    QByteArray ENQ = QString(5).toLocal8Bit();
    QByteArray ACK = QString(6).toLocal8Bit();
    QByteArray NAK = QString(15).toLocal8Bit();    
    QByteArray requestData = "";
    QByteArray responseData = "";   
    QByteArray buffer = "";
    int bytesWritten;    
    bool Recing = false;    

    //write and read
    while(!quit){
        //cond.wakeAll(); //解鎖
        cmding = PlcCommand; //避免 PlcCommand 被更改時，引起判斷上的錯誤
        PlcCommand = Read_M; //default
        RxCount = 0;

        //encode
        switch (cmding)
        {
            case M100_ON:
                requestData = ENQ + station + PC + "BW5M01000115A";
                break;
            case M100_OFF:
                requestData = ENQ + station + PC + "BW5M010001059";
                break;
            default: //0 Read_M
                requestData = ENQ + station + PC + "BR5M01000427"; //read M100~M103
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

            // read response 抓資料
            if (serial.waitForReadyRead(50000)) { //若一直沒接收到，會在這等待5秒
                responseData = serial.readAll(); //接收到後抓近來
                emit status(">responseData:" + responseData + " - " +responseData.toHex());

                for(int i=0 ; i < responseData.length() ; i++){
                    if(responseData.mid(i,1) == STX && !Recing){
                        buffer = responseData.mid(i,1);
                        Recing = true;
                    }else if(responseData.mid(i,1) == ETX && Recing){
                        buffer += responseData.mid(i,1);
                        Recing = false;

                        //output
                        process(buffer);

                        //回答ACK了解
                        serial.write(ACK + station + PC);
                        if(!serial.waitForBytesWritten(5000))
                            emit status(QString("waitForBytesWritten() timed out for port %1, error: %2").
                                arg(serial.portName()).arg(serial.errorString()));

                        buffer = "";

                    }else if(responseData.mid(i,1) == ACK || responseData.mid(i,1) == NAK){
                        buffer = "";
                        Recing = false;
                    }else if(Recing){
                        buffer += responseData.mid(i,1);
                    }
                }

            }else{
                emit status("Wait read response timeout");
            }

            //撿查接收後 是否有錯誤
            if (serial.error() == QSerialPort::ReadError)
                emit status(QString("Failed to read from port %1, error: %2").
                            arg(serial.portName()).arg(serial.errorString()));

        }else{
            emit status(QString("waitForBytesWritten() timed out for port %1, error: %2").
                    arg(serial.portName()).arg(serial.errorString()));
        }

        //結果
        emit status(">\n buffer:" + buffer + " " + buffer.toHex()
                    + "\n =======================");

        msleep(100);
    }

    if (serial.isOpen())
        serial.close();
    emit status("0");
}


