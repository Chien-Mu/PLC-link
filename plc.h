#ifndef PLC_H
#define PLC_H

#include <QThread>
#include <qDebug>
#include <QMutex>
#include <QWaitCondition>

typedef enum{
    X0_ON = 0,
    X0_OFF
}PLC_Node;

class PLC : public QThread{
    Q_OBJECT
public:
    PLC();
    ~PLC();
    void run();
    void setCOM(QString COM_ID, int DelayTime, int Timeout);

public slots:
    void serial_write(QByteArray value);
    void stop();

private:
    bool quit;
    QMutex mutex;
    QWaitCondition cond;
    QByteArray requestData;
    int bytesWritten;

    //set
    QString COM_ID;
    int DelayTime;
    int Timeout;

signals:
    void status(QString value);

};

#endif // PLC_H
