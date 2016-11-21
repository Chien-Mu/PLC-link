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

    //plc
    plc = new PLC();
    connect(this,SIGNAL(plc_quit()),plc,SLOT(stop())); //解除serialport
    connect(plc,SIGNAL(status(QString)),this,SLOT(setStatus(QString))); //thread打過來的    
    connect(ui->btn_open,SIGNAL(clicked()),this,SLOT(openSerialPort_thread())); //open
    connect(ui->btn_close,SIGNAL(clicked()),this,SLOT(closeSerialPort_thread())); //close
    connect(ui->btn_x0_on,SIGNAL(clicked()),this,SLOT(X0_NO_click()));
    connect(ui->btn_x0_off,SIGNAL(clicked()),this,SLOT(X0_OFF_click()));
}

void MainWindow::X0_NO_click(){
    plc->cmd(M100_ON);
}
void MainWindow::X0_OFF_click(){
    plc->cmd(M100_OFF);
}


void MainWindow::openSerialPort_thread(){
    if(!plc->isRunning()){
        plc->setCOM(ui->comboBox->currentText(),0,0);
        plc->start(); //Start
    }
}

void MainWindow::closeSerialPort_thread(){
    emit plc_quit();
}

void MainWindow::getSerialInfo(){
    //列出系統資訊
    ui->comboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        ui->comboBox->addItem(info.portName());
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
