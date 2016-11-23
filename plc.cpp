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

    PlcCommand = Read_M; //default
    PLC_CMD cmding = Read_M;
    QByteArray station = "00";
    QByteArray PC = "FF";
    QByteArray STX = QString(2).toLocal8Bit();
    QByteArray ETX = QString(3).toLocal8Bit();
    QByteArray ENQ = QString(5).toLocal8Bit();
    QByteArray ACK = QString(6).toLocal8Bit();
    QByteArray test = "T";
    QByteArray requestData,responseData,HeadDate,TailData;
    int bytesWritten;
    int RxCount,Headlength,Taillength;
    bool RxOK = false;

    //write and read
    while(!quit){
        //cond.wakeAll(); //解鎖
        cmding = PlcCommand; //避免 PlcCommand 被更改時，引起判斷上的錯誤
        PlcCommand = Read_M; //default

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
            if (serial.waitForReadyRead(5000)) { //若一直沒接收到，會在這等待5秒

                //上筆是否有先抓的資料
                if(TailData.length() > 0){
                    responseData = TailData;
                    TailData = ""; //default
                    responseData += serial.readAll(); //接收到後抓近來
                }else
                    responseData = serial.readAll();

                //抓殘留，等10ms
                while (serial.waitForReadyRead(10))
                    responseData += serial.readAll();

                if(cmding == Read_M){
                    //接收到ETX 才會離開，5次機會
                    RxCount = 0;
                    RxOK = false;
                    do{
                        if(serial.waitForReadyRead(50))
                            responseData += serial.readAll();
                        if(responseData.contains(ETX))
                            RxOK = true;
                        else
                            RxCount++;
                    }while (!RxOK && RxCount < 5); //任一項達成都會跳離

                    //將檢查碼後多餘的接收另存
                    if(RxOK){
                        Headlength = responseData.indexOf(ETX) + 3;
                        Taillength = responseData.length() - Headlength;
                        HeadDate = responseData.left(Headlength);
                        TailData = responseData.right(Taillength);

                        //回答ACK了解
                        serial.write(ACK + station + PC);
                        if(!serial.waitForBytesWritten(5000))
                            emit status(QString("waitForBytesWritten() timed out for port %1, error: %2").
                                arg(serial.portName()).arg(serial.errorString()));

                    }else{
                        emit status(QString::fromLocal8Bit("傳送 ENQ 後，未讀取到 ETX"));
                    }
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

        //結果
        emit status(">HeadDate:" + HeadDate + " " +HeadDate.toHex()
                    + "\n TailData:" + TailData + " " + TailData.toHex()
                    + "\n responseData:" + responseData + " " + responseData.toHex()
                    + "\n RxCount:" + QString::number(RxCount));

        //output
        if(HeadDate.at(5) == '0')
            emit M100(false);
        else if(HeadDate.at(5) == '1')
            emit M100(true);

        msleep(100);
    }

    if (serial.isOpen())
        serial.close();
    emit status("0");
}


