#include "plc.h"
#include <QtSerialPort/QSerialPort>

PLC::PLC(){
    this->quit = false;
    this->selectIndex = -1;
    this->R_packet.isNull = true;

    /*
    //read M100~M103 範例
    this->packet.ident = 0;
    this->packet.station = "00";
    this->packet.PC = "FF";
    this->packet.R_or_W = "BR";
    this->packet.wait = "5";
    this->packet.index = "M0100";
    this->packet.count = "04";
    this->packet.Data = "";
    this->packet.check = "27";
    */
}

PLC::~PLC(){
    this->quit = true;
}

void PLC::setCOM(QString COM_ID, int DelayTime, int Timeout){
    this->COM_ID = COM_ID;
    this->DelayTime = DelayTime;
    this->Timeout = Timeout;
}

void PLC::setRead(PLC_Request &value){
    this->R_packet = value;
    this->R_packet.isNull = false;
}

void PLC::setWrite(QVector<PLC_Request> &value){
    this->W_packet = value;
}

void PLC::triggerWrite(unsigned index){
    mutex.lock();
    this->selectIndex = index;
    mutex.unlock();
}

void PLC::stop(){
    quit = true;
}

bool PLC::process(QByteArray value, int DataTotal){
    /*這裡只有 STX 開頭 結尾為 ETX 的字串才會進來*/
    bool isError = false;

    //檢查
    if(value.length() != (6 + DataTotal) ){
        //6 是必要的字元數，DataTotal 是要求時有幾組

        /* 假設正常資料進來是 13 個字元，檢查設定要 == 13，才是合格
         * 因為有時會回傳 value: "\x02""00FFpA\r\"\x0B\x7F""00FF0101000\x03" 垃圾與訊號都會包在一起
         * 而且會發很多次過一會才會給 value: "\x02""00FF0101000\x03" 正確的值
         * 所以要利用 大於13的來判斷是否有誤*/
        return true;
    }

    //取出Data
    QByteArray data = value.mid(5,DataTotal); //取出Data，5是 STX00FF 後的index
    QRegExp rx("^[01]+$"); //0、1
    if (rx.indexIn(data) == 0) //確認那四碼只有 0、1
        emit RequestData(data); //送出
    else
        isError = true;

    return isError;
}

void PLC::run(){
    quit = false; //這裡要初始化，不然第二次執行時，他會殘留上次執行的值
    this->selectIndex = -1;
    //=========================================================

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

    int _index = -1;    //default
    QByteArray STX = QString(2).toLocal8Bit();
    QByteArray ETX = QString(3).toLocal8Bit();
    QByteArray ENQ = QString(5).toLocal8Bit();
    QByteArray ACK = QString(6).toLocal8Bit();
    QByteArray NAK = QString(21).toLocal8Bit(); //十進制是21 16進制是15
    PLC_Request convStr;
    QByteArray requestData = "";
    QByteArray responseData = "";   
    QByteArray buffer = "";
    int bytesWritten;    
    bool Recing = false;    

    //write and read
    while(!quit){
        if(this->W_packet.size() == 0) //一定要有Write的值才執行
            break;


        _index = this->selectIndex; //避免下面使用 selectIndex 時，突然 selectIndex 被更改的形況
        this->selectIndex = -1;     //馬上 default

        //encode
        if(_index == -1){   //要求讀取
            if(this->R_packet.isNull)   //如果選擇讀取，但沒讀取的資料
                break;
            else
                convStr = this->R_packet;
        }else{              //要求設定
            if(_index < this->W_packet.size())  //如果 triggerWrite 的值超出範圍就跳離迴圈
                convStr = this->W_packet[_index];
            else
                break;

            _index = -1;    //Write 後自動回到 Read
        }

        //conver char
        requestData = ENQ +
                      convStr.station +
                      convStr.PC +
                      convStr.R_or_W +
                      convStr.wait +
                      convStr.index +
                      convStr.count +
                      convStr.Data +
                      convStr.check;

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
        if(serial.waitForBytesWritten(Timeout)){

            // read response 抓資料
            if (serial.waitForReadyRead(Timeout)) { //若一直沒接收到，會在這等待5秒
                responseData = serial.readAll(); //接收到後抓近來

                for(int i=0 ; i < responseData.length() ; i++){
                    if(responseData.mid(i,1) == STX && !Recing){ //STX 且 not Recing
                        buffer = responseData.mid(i,1);
                        Recing = true;
                    }else if(responseData.mid(i,1) == ETX && Recing){ //ETX 且 Recing
                        buffer += responseData.mid(i,1);
                        Recing = false;

                        //output
                        if(process(buffer,convStr.count.toInt())){
                            serial.write(NAK + convStr.station + convStr.PC);  //回答NAK有問題
                            emit status("Requested, Packet error.Packet: STX " + buffer + " ETX");
                        }else
                            serial.write(ACK + convStr.station + convStr.PC);  //回答ACK了解

                        if(!serial.waitForBytesWritten(Timeout))
                            emit status(QString("waitForBytesWritten() timed out for port %1, error: %2").
                                arg(serial.portName()).arg(serial.errorString()));

                        buffer = ""; //process後就全部清除，因為是字元一個一個進來，所以不會影響後面的字元
                    }else if(responseData.mid(i,1) == ACK || responseData.mid(i,1) == NAK){
                        buffer = "";
                        Recing = false;
                    }else if(responseData.mid(i,1) != ETX && responseData.mid(i,1) != STX && Recing){ //累加
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
        //emit status(">\n buffer:" + buffer + " " + buffer.toHex()
        //            + "\n =======================");

        msleep(DelayTime);
    }

    this->R_packet.isNull = true;
    this->W_packet.clear(); //清空
    //this->W_packet.squeeze(); //清除記憶體
    //this->W_packet.swap(QVector<PLC_Request>()); //清除記憶體

    if (serial.isOpen())
        serial.close();
    emit status("0");
}


