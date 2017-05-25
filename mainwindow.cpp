#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QtSerialPort/QSerialPortInfo>

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

    //===================PLC 使用範例===================
    //建立 PLC object
    plc = new PLC;
    //設定開始與結束
    connect(ui->btn_open,SIGNAL(clicked()),this,SLOT(Start())); //PLC object 設定與啟動寫在 MainWindow::Start()
    connect(ui->btn_close,SIGNAL(clicked()),this,SLOT(Stop()));  //PLC object 關閉寫在 MainWindow::Stop()
    //狀況回報
    connect(plc,SIGNAL(status(QString)),this,SLOT(setStatus(QString))); //PLC object 的 status() Singal 會即時回傳error 等狀況
    //設定X0的命令範例
    connect(ui->btn_m100_on,SIGNAL(clicked()),this,SLOT(M100_NO_click()));  //M100 設定 ON  寫在 MainWindow::M100_NO_click()
    connect(ui->btn_m100_off,SIGNAL(clicked()),this,SLOT(M100_OFF_click()));//M100 設定 OFF 寫在 MainWindow::M100_OFF_click()
    //取得讀取資料範例
    connect(plc,SIGNAL(RequestData(QByteArray)),this,SLOT(showLED(QByteArray))); //要求讀取的資料由 Signal RequestData(QByteArray) 丟出
    //=================================================
}

void MainWindow::M100_NO_click(){
    plc->triggerWrite(1);       //觸發 "ENQ00FFBW5M01000115A"
}
void MainWindow::M100_OFF_click(){
    plc->triggerWrite(0);       //觸發 "ENQ00FFBW5M010001059"
}

void MainWindow::Start(){
    /* PLC object 設定與啟動範例 */

    if(!plc->isRunning()){      //在未啟動狀態 PLC object 才能啟動與設定
        //設定 COM，setCOM(COM名稱, 執行週期Delay時間(單位:ms), COM Timeout(單位:ms))
        plc->setCOM(ui->comboBox->currentText(),100,20000);

        //設定"週期性 讀取PLC"的讀取命令。
        /* "ENQ00FFBRAM01000130" 讀取 M100 作為範例 */
        PLC_Request ReadCmd;
        ReadCmd.station = "00";
        ReadCmd.PC = "FF";
        ReadCmd.R_or_W = "BR";
        ReadCmd.wait = "A";
        ReadCmd.index = "M0100";
        ReadCmd.count = "01";
        ReadCmd.Data = "";
        ReadCmd.check = "30";
        plc->setRead(ReadCmd);              //加入至 PLC object

        //設定 "寫入PLC"的資料，支援多組資料。
        /* 加入兩組為範例
         * "ENQ00FFBW5M010001059" 設定 M100 OFF
         * "ENQ00FFBW5M01000115A" 設定 M100 ON */
        /* 執行時再由 triggerWrite() 方法指定執行的設定命令
         * 指定設定命令後，週期循環第二次時若沒指定，則會自動切換成以"讀取PLC"讀取命令執行 */
        QVector<PLC_Request> WriteCmd_list;
        PLC_Request WriteCmd;
        WriteCmd.station = "00";
        WriteCmd.PC = "FF";
        WriteCmd.R_or_W = "BW";
        WriteCmd.wait = "5";
        WriteCmd.index = "M0100";
        WriteCmd.count = "01";
        WriteCmd.Data = "0";
        WriteCmd.check = "59";
        WriteCmd_list.push_back(WriteCmd);  //加入 "ENQ00FFBW5M010001059"，此為 index 0
        WriteCmd.Data = "1";
        WriteCmd.check = "5A";
        WriteCmd_list.push_back(WriteCmd);  //加入 "ENQ00FFBW5M01000115A"，此為 index 1
        plc->setWrite(WriteCmd_list);       //加入至 PLC object

        //啟動
        plc->start();
    }
}

void MainWindow::Stop(){
    /* PLC object 關閉範例 */
    // PLC::stop() 也是 SLOT，所以其實也可以直接利用 Signal 連接它
    plc->stop();
}

void MainWindow::showLED(QByteArray data){
    if(data.at(0) == '1')
        ui->LE_m100->setPalette(bright);
    else
        ui->LE_m100->setPalette(Dark);
}

void MainWindow::setStatus(QString value){
    if(value == "0"){
        status->setText("Not connected");
    }else if(value == "1"){
        QMessageBox::information(this,"OPEN",QString::fromLocal8Bit("成功連接"));
        status->setText("Connected");
    }else if(value.at(0) == '>'){
        ui->pte_receive->appendPlainText(value);
    }else{
        QMessageBox::critical(this, tr("Error"), value);
    }
}

void MainWindow::getSerialInfo(){
    //載入系統COM資訊
    ui->comboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        ui->comboBox->addItem(info.portName());
}

MainWindow::~MainWindow()
{
    delete ui;
}
