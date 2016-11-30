#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    //ui
    status = new QLabel;
    ui->statusBar->addWidget(status);
    status->setText("Not connected");
    getSerialInfo();
    bright.setColor(QPalette::Base,Qt::green);
    Dark.setColor(QPalette::Base,Qt::darkGreen);
    ui->LE_m100->setPalette(Dark);

    //plc
    plc = new PLC();           
    connect(ui->btn_open,SIGNAL(clicked()),this,SLOT(openSerialPort_thread())); //open
    connect(ui->btn_close,SIGNAL(clicked()),plc,SLOT(stop())); //close
    connect(plc,SIGNAL(status(QString)),this,SLOT(setStatus(QString))); //plc 打回來的資訊
    connect(ui->btn_x0_on,SIGNAL(clicked()),this,SLOT(X0_NO_click())); //命令
    connect(ui->btn_x0_off,SIGNAL(clicked()),this,SLOT(X0_OFF_click()));
    connect(plc,SIGNAL(M100(bool)),this,SLOT(showLED(bool))); //M100
}

void MainWindow::X0_NO_click(){
    plc->cmd(M100_ON);
}
void MainWindow::X0_OFF_click(){
    plc->cmd(M100_OFF);
}

void MainWindow::openSerialPort_thread(){
    if(!plc->isRunning()){
        plc->setCOM(ui->comboBox->currentText(),100,0);
        plc->start(); //Start
    }
}

void MainWindow::getSerialInfo(){
    //列出系統資訊
    ui->comboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        ui->comboBox->addItem(info.portName());
}

void MainWindow::showLED(bool value){
    if(value)
        ui->LE_m100->setPalette(bright);
    else
        ui->LE_m100->setPalette(Dark);
}

void MainWindow::setStatus(QString value){
    if(value == "0"){
        status->setText("Not connected");
    }else if(value == "1"){
        status->setText("Connected");
    }else if(value.at(0) == '>'){
        ui->pte_receive->appendPlainText(value);
    }else{
        QMessageBox::critical(this, tr("Error"), value);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
