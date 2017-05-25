#ifndef PLC_H
#define PLC_H

#include <QThread>
#include <qDebug>
#include <QVector>
#include <QMutex>

struct PLC_Request{
    bool isNull;        //要求設定不會用到
    int ident;          //編號(要求讀取時不會用到)
    QByteArray station; //站號
    QByteArray PC;      //PC
    QByteArray R_or_W;  //指令型態
    QByteArray wait;    //等待時間
    QByteArray index;   //起始處
    QByteArray count;   //共有幾組
    QByteArray Data;    //Write data(要求讀取時不會用 data)
    QByteArray check;   //檢查碼
};

class PLC : public QThread{
    Q_OBJECT
public:
    PLC();
    ~PLC();
    void run();
    void setCOM(QString COM_ID, int DelayTime, int Timeout);
    void setRead(PLC_Request &value);
    void setWrite(QVector<PLC_Request> &value);
    void triggerWrite(unsigned index);

public slots:   
    void stop();

private:
    bool quit;
    QMutex mutex;
    PLC_Request packet;
    bool process(QByteArray value, int DataTotal);

    //set
    QString COM_ID;
    int DelayTime;
    int Timeout;

    //Packet
    int selectIndex;
    PLC_Request R_packet;
    QVector<PLC_Request> W_packet;

signals:
    void status(QString value);
    void RequestData(QByteArray data);

};

#endif // PLC_H
