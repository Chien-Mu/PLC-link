#ifndef PLC_H
#define PLC_H

#include <QThread>
#include <qDebug>
#include <QVector>
#include <QMutex>

struct PLC_Request{
    bool isNull;        //要求設定不會用到
    int ident;          //編號(要求讀取時不會用到)
    QByteArray station;
    QByteArray PC;
    QByteArray R_or_W;
    QByteArray wait;
    QByteArray index;
    QByteArray count;
    QByteArray Data;    //要求讀取時不會用 data
    QByteArray check;
};

class PLC : public QThread{
    Q_OBJECT
public:
    PLC();
    ~PLC();
    void run();
    void setCOM(QString COM_ID, int DelayTime, int Timeout);
    void setRead(PLC_Request value);
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
