#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

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
    QPalette bright;
    QPalette Dark;
    void getSerialInfo();
    PLC *plc;

private slots:
    void Start();
    void Stop();
    void setStatus(QString value);
    void M100_NO_click();
    void M100_OFF_click();
    void showLED(QByteArray data);

};

#endif // MAINWINDOW_H
