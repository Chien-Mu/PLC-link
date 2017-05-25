# PLC Class #

* PLC Class 主要用於 PC 與 PLC 溝通用
* 協定使用 三菱 RS485/RS232C

## PLC Class 如何加入 ##

* qmake: QT += serialport
* Files: plc.h、plc.cpp
* include: #include "plc.h"

## PLC Class 架構 ##

1.  Struction PLC_Request (封包結構) 
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


# 範例簡介 #

* 裡面有 PLC Class 與 MainWindow Class, MainWindow 主要是要示範如何使用 PLC Class.

* 了解使用 PLC Class ,不需要去看 PLC Class 內容,只需要看 MainWindow Class 使用 PLC Class 範例即可知道如何運用.