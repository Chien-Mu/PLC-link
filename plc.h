#ifndef PLC_H
#define PLC_H

#include <QThread>
#include <qDebug>
#include <QMutex>
#include <QWaitCondition>

typedef enum{
    Read_M = 0,
    M100_ON,
    M100_OFF
}PLC_CMD;

class PLC : public QThread{
    Q_OBJECT
public:
    PLC();
    ~PLC();
    void run();
    void setCOM(QString COM_ID, int DelayTime, int Timeout);
    void cmd(PLC_CMD PlcCommand);

public slots:   
    void stop();

private:
    bool quit;
    QMutex mutex;
    QWaitCondition cond;
    PLC_CMD PlcCommand; //只能由 cmd()設定，其餘地方不能設定，以防止PlcCommand命令遺漏

    //set
    QString COM_ID;
    int DelayTime;
    int Timeout;

signals:
    void status(QString value);

};

#endif // PLC_H
