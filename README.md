# PLC Class 簡介 #

* PLC Class 主要用於 PC 與 PLC 溝通用
* 協定使用 三菱 RS485/RS232C



## 範例簡介 ##

* 裡面有 PLC Class 與 MainWindow Class, MainWindow 主要是要示範如何使用 PLC Class.

* 了解使用 PLC Class ,不需要去看 PLC Class 內容,只需要看 MainWindow Class 使用 PLC Class 範例即可知道如何運用.



## PLC Class 邏輯簡介 ##

* 使用 PC 與 PLC 通訊往往需要一個執行緒在背後持續循環讀取目前 PLC 的狀態，
也需要多組設定 PLC 不同狀態的功能，PLC Class 就是以此模式建立出來的類別。

* 先將一組循環讀取PLC狀態的命令、多組設定PLC狀態的命令，加入PLC object後，
執行 PLC::start()，之後會自動循環執行 PLC::setRead() 設定的讀取命令。  
讀到的檔案會從 RequestData(QByteArray data) Signal 丟出。  

* 執行 PLC::start()後，執行 PLC::triggerWrite(index) 會執行 trigger 的設定PLC狀態命令。(一次 triggerWrite() 只會執行一次寫入)



### PLC Class 如何加入 ###

* qmake: QT += serialport
* Files: plc.h、plc.cpp
* include: #include "plc.h"



### PLC_Request Struction ###

*  bool isNull;        //要求設定不會用到
*  int ident;          //編號(要求讀取時不會用到)
*  QByteArray station; //站號
*  QByteArray PC;      //PC
*  QByteArray R_or_W;  //指令型態
*  QByteArray wait;    //等待時間
*  QByteArray index;   //起始處
*  QByteArray count;   //共有幾組
*  QByteArray Data;    //Write data(要求讀取時不會用 data)
*  QByteArray check;   //檢查碼



### Signals ###

*  void PLC::status(QString value);  
傳遞當下狀態

*  void PLC::RequestData(QByteArray data);  
傳遞讀取指令收到的data(不包含 STX、站號、PC、ETX、檢查碼)



### Slots ###

*  void PLC::stop();  
停止循環

*  void PLC::start();  
啟動循環  
**啟動循環前**，務必先設定:setCOM、setRead、setWrite，之後再啟動。  
**啟動循環後**，切勿設定:setCOM、setRead、setWrite，因執行緒安全保護原因，會將它們視為無效設定，必須先執行stop()後才設定它們。
```
#!c++
example:
if(!plc->isRunning()){ 
    plc->setCOM( ... );
    plc->setRead( ... );
    plc->setWrite( ... );
    plc->start();
}
```



### Functions ###

*  void PLC::setCOM(QString COM_ID, int DelayTime, int Timeout);  
設定COM，setCOM(COM名稱, 執行週期Delay時間(單位:ms), COM Timeout(單位:ms))

*  void PLC::setRead(PLC_Request &value);  
設定"週期性 讀取PLC"的讀取命令

*  void PLC::setWrite(QVector<PLC_Request> &value);  
設定 "寫入PLC"的資料  
**支援多組資料，因此使用 QVector<PLC_Request>**

*  void PLC::triggerWrite(unsigned index);  
執行時由 triggerWrite() 方法指定執行的設定命令.  
指定設定命令後，**週期循環第二次時若沒指定，則會自動切換成以"讀取PLC"讀取命令執行.(一次 triggerWrite() 只會執行一次寫入)**