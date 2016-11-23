#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "plc.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QLabel *status;
    QSerialPort *serial;
    PLC *plc;
    QPalette bright;
    QPalette Dark;

private slots:
    void getSerialInfo();
    void openSerialPort_thread();
    void closeSerialPort_thread();
    void X0_NO_click();
    void X0_OFF_click();
    void showLED(bool value);


public slots:
    void setStatus(QString value);

signals:
    void plc_quit();
};

#endif // MAINWINDOW_H
