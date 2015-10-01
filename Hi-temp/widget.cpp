#include "widget.h"
#include "ui_widget.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QTextCodec>

using namespace YExcel;

#define POLY                0x1021  //CRC生成多项式
#define Segment_Low         1       //段数的下限
#define Segment_High        30      //段数的上限
#define SuperID_0           0xAA    //超级ID0
#define SuperID_1           0xAA    //超级ID1
#define SuperID_2           0x55    //超级ID2
#define SuperID_3           0x55    //超级ID3
//#define SegmentDiff         30      //对应的实际电压和线性电压固定地址差
#define FixBaud             38400   //串口固定波特率

/* 各个模块寄存器的数量
 */
#define IDRegNum            2       //出厂编号寄存器个数
#define LineRegNum          61      //线性化寄存器个数
#define ADcaliRegNum        4       //AD校验寄存器个数
#define TRegNum             2       //温度寄存器个数
#define VcaliRegNum         5       //电压校验寄存器个数
#define AcaliRegNum         4       //电流校验寄存器个数
#define EMISRegNum          2       //发射率寄存器个数
#define PeakRegNum          4       //峰值寄存器个数
#define AlarmLowRegNum      3       //低温报警寄存器个数
#define AlarmHighRegNum     3       //高温报警寄存器个数
#define ResTimeRegNum       1       //响应时间寄存器个数


/* 寄存器的地址
 */
#define ADD_DeviceIDRead    0       //读取ID寄存器
#define ADD_DeviceID1       0       //ID寄存器1
#define ADD_DeviceID2       1       //ID寄存器1

#define ADD_LineRead        2       //读取线性化寄存器
#define ADD_Line            2       //线性化寄存器首地址
#define ADD_LineRealStart   2       //实际电压寄存器首地址
#define ADD_LineRealEnd     31      //实际电压寄存器尾地址
#define ADD_LineLineStart   32      //线性电压寄存器首地址
#define ADD_LineLineEnd     61      //线性电压寄存器尾地址
#define ADD_SegmentNum      62      //线性化点数

#define ADD_ADcaliRead      63      //读取AD校验
#define ADD_ADcaliDLow      63      //AD校验低电压对应D
#define ADD_ADcaliDHigh     64      //AD校验高电压对应D
#define ADD_ADcaliVLow      65      //AD校验低电压
#define ADD_ADcaliVHigh     66      //AD校验高电压

#define ADD_TRangeRead      67      //读取温度
#define ADD_TRangeLow       67      //温度下限
#define ADD_TRangeHigh      68      //温度上限

#define ADD_VcaliRead       69      //读取电压校验(DAC0 OUT0)
#define ADD_VcaliDLow       69      //电压校验D值下限
#define ADD_VcaliDHigh      70      //电压校验D值上限
#define ADD_VcaliVLow       71      //电压校验下限电压
#define ADD_VcaliVHigh      72      //电压校验上限电压
#define ADD_VcaliD          73      //直接输出电压D值

#define ADD_EMISRead        74      //读取发射率
#define ADD_EMISFull        74      //发射率满度电压
#define ADD_EMIS            75      //发射率


#define ADD_AcaliRead       76      //读取电流校验D值
#define ADD_AcaliOn         76      //电流校验开关
#define ADD_AcaliD          77      //电流校验D值输出
#define ADD_AcaliDLow       78      //电流校验D值下限
#define ADD_AcaliDHigh      79      //电流校验D值上限

#define ADD_PeakRead        80      //读取峰值
#define ADD_PeakOn          80      //峰值开关
#define ADD_PeakThr         81      //峰值阈值
#define ADD_HoldTime        82      //保持时间
#define ADD_DeRate          83      //衰减率

#define ADD_AlarmLowRead    84      //读取低温报警
#define ADD_AlarmLowOn      84      //低温报警开关
#define ADD_AlarmLowThr     85      //低温报警阈值
#define ADD_AlarmLowHy      86      //低温回差
#define ADD_AlarmHighRead   87      //读取高温警报
#define ADD_AlarmHighOn     87      //高温报警开关
#define ADD_AlarmHighThr    88      //高温报警阈值
#define ADD_AlarmHighHy     89      //高温回差

#define ADD_ResTimeRead     90      //读取响应时间
#define ADD_ResTime         90      //响应时间

#define ADD_Password        91      //密码


/* 功能码
 */
#define Function_UpdateFlash    255     //MCU更新FLASH
#define Function_Inquire        3       //查询功能码
#define Function_Modify         6       //修改功能码

/* 信号
 */
#define SIGNAL_ADcaliLow        1       //AD低校验
#define SIGNAL_ADcaliHigh       2       //AD高校验
#define SIGNAL_ADcaliRead       3       //读取AD校验
#define SIGNAL_LineWrite        4       //写入线性化数据
#define SIGNAL_LineRead         5       //读取线性化数据
#define SIGNAL_segment_Error    6       //段数输入错误
#define SIGNAL_Open_Error       7       //串口开启错误
#define SIGNAL_IDWrite          8       //写入出厂编号
#define SIGNAL_IDRead           9       //读取出厂编号
#define SIGNAL_TWrite           10      //写入温度
#define SIGNAL_TRead            11      //读取温度
#define SIGNAL_VcaliD           14      //电压校验输出电压D值
#define SIGNAL_VcaliVLow        15      //电压校验下限电压
#define SIGNAL_VcaliVHigh       16      //电压校验上限电压
#define SIGNAL_VcaliRead        17      //读取电压校验
#define SIGNAL_VcaliClear       18      //清空电压校验
#define SIGNAL_EMISWrite        19      //写入发射率
#define SIGNAL_EMISRead         20      //读取发射率
#define SIGNAL_EMISClear        21      //清空发射率
#define SIGNAL_PeakWrite        22      //写入峰值保持
#define SIGNAL_PeakRead         23      //读取峰值保持
#define SIGNAL_PeakClear        24      //清空峰值保持
#define SIGNAL_AlarmLowWrite    25      //写入低温报警
#define SIGNAL_AlarmLowRead     26      //读取低温报警
#define SIGNAL_AlarmLowClear    27      //清空低温报警
#define SIGNAL_AlarmHighWrite   28      //写入高温报警
#define SIGNAL_AlarmHighRead    29      //读取高温报警
#define SIGNAL_AlarmHighClear   30      //清空高温报警
#define SIGNAL_ResTimeWrite     31      //写入响应时间
#define SIGNAL_ResTimeRead      32      //读取响应时间
#define SIGNAL_ResTimeClear     33      //清空响应时间
#define SIGNAL_AcaliOn          34      //进入电流校验
#define SIGNAL_AcaliOff         35      //退出电流校验
#define SIGNAL_AcaliDLow        36      //设置电流校验下限
#define SIGNAL_AcaliDHigh       37      //设置电流校验上限
#define SIGNAL_AcaliOut         38      //电流校验D值输出
#define SIGNAL_AcaliRead        39      //读取电流校验
#define SIGNAL_AcaliClear       40      //清空电流校验
#define SIGNAL_EMIS_Error       41      //发射率百分比超过100
#define SIGNAL_Excel_Format     42      //Excel格式错误
#define SIGNAL_Excel_NoFile     43      //Excel文件不存在

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);


    //成员变量初始化
    ID = 0;
    TXcnt = 0;
    RXcnt = 0;
    SuperID[0] = SuperID_0;
    SuperID[1] = SuperID_1;
    SuperID[2] = SuperID_2;
    SuperID[3] = SuperID_3;

    //控件关联变量
    LineEdit_real[0] = ui->lineEdit_real1;
    LineEdit_real[1] = ui->lineEdit_real2;
    LineEdit_real[2] = ui->lineEdit_real3;
    LineEdit_real[3] = ui->lineEdit_real4;
    LineEdit_real[4] = ui->lineEdit_real5;
    LineEdit_real[5] = ui->lineEdit_real6;
    LineEdit_real[6] = ui->lineEdit_real7;
    LineEdit_real[7] = ui->lineEdit_real8;
    LineEdit_real[8] = ui->lineEdit_real9;
    LineEdit_real[9] = ui->lineEdit_real10;
    LineEdit_real[10] = ui->lineEdit_real11;
    LineEdit_real[11] = ui->lineEdit_real12;
    LineEdit_real[12] = ui->lineEdit_real13;
    LineEdit_real[13] = ui->lineEdit_real14;
    LineEdit_real[14] = ui->lineEdit_real15;
    LineEdit_real[15] = ui->lineEdit_real16;
    LineEdit_real[16] = ui->lineEdit_real17;
    LineEdit_real[17] = ui->lineEdit_real18;
    LineEdit_real[18] = ui->lineEdit_real19;
    LineEdit_real[19] = ui->lineEdit_real20;
    LineEdit_real[20] = ui->lineEdit_real21;
    LineEdit_real[21] = ui->lineEdit_real22;
    LineEdit_real[22] = ui->lineEdit_real23;
    LineEdit_real[23] = ui->lineEdit_real24;
    LineEdit_real[24] = ui->lineEdit_real25;
    LineEdit_real[25] = ui->lineEdit_real26;
    LineEdit_real[26] = ui->lineEdit_real27;
    LineEdit_real[27] = ui->lineEdit_real28;
    LineEdit_real[28] = ui->lineEdit_real29;
    LineEdit_real[29] = ui->lineEdit_real30;

    LineEdit_line[0] = ui->lineEdit_line1;
    LineEdit_line[1] = ui->lineEdit_line2;
    LineEdit_line[2] = ui->lineEdit_line3;
    LineEdit_line[3] = ui->lineEdit_line4;
    LineEdit_line[4] = ui->lineEdit_line5;
    LineEdit_line[5] = ui->lineEdit_line6;
    LineEdit_line[6] = ui->lineEdit_line7;
    LineEdit_line[7] = ui->lineEdit_line8;
    LineEdit_line[8] = ui->lineEdit_line9;
    LineEdit_line[9] = ui->lineEdit_line10;
    LineEdit_line[10] = ui->lineEdit_line11;
    LineEdit_line[11] = ui->lineEdit_line12;
    LineEdit_line[12] = ui->lineEdit_line13;
    LineEdit_line[13] = ui->lineEdit_line14;
    LineEdit_line[14] = ui->lineEdit_line15;
    LineEdit_line[15] = ui->lineEdit_line16;
    LineEdit_line[16] = ui->lineEdit_line17;
    LineEdit_line[17] = ui->lineEdit_line18;
    LineEdit_line[18] = ui->lineEdit_line19;
    LineEdit_line[19] = ui->lineEdit_line20;
    LineEdit_line[20] = ui->lineEdit_line21;
    LineEdit_line[21] = ui->lineEdit_line22;
    LineEdit_line[22] = ui->lineEdit_line23;
    LineEdit_line[23] = ui->lineEdit_line24;
    LineEdit_line[24] = ui->lineEdit_line25;
    LineEdit_line[25] = ui->lineEdit_line26;
    LineEdit_line[26] = ui->lineEdit_line27;
    LineEdit_line[27] = ui->lineEdit_line28;
    LineEdit_line[28] = ui->lineEdit_line29;
    LineEdit_line[29] = ui->lineEdit_line30;

    LineEdit_lineNum = ui->lineEdit_lineNum;

    TextEdit_History = ui->textEdit_ComHistory;

    //峰值Radio
    btnGroupPeak = new QButtonGroup(this);
    btnGroupPeak->addButton(ui->radioButton_PeakOff, 0);
    btnGroupPeak->addButton(ui->radioButton_PeakOn, 1);
    ui->radioButton_PeakOff->setChecked(true);
    ui->lineEdit_PeakThr->setVisible(false);
    ui->label_PeakThr->setVisible(false);
    ui->lineEdit_HoldTime->setVisible(false);
    ui->label_HoldTime->setVisible(false);
    ui->lineEdit_DeRate->setVisible(false);
    ui->label_DeRate->setVisible(false);
    connect(ui->radioButton_PeakOff, SIGNAL(clicked()), this, SLOT(onRadioClickPeak()));
    connect(ui->radioButton_PeakOn, SIGNAL(clicked()), this, SLOT(onRadioClickPeak()));

    //低温警报Radio
    btnGroupAlarmLow = new QButtonGroup(this);
    btnGroupAlarmLow->addButton(ui->radioButton_AlarmLowOff, 0);
    btnGroupAlarmLow->addButton(ui->radioButton_AlarmLowOn, 1);
    ui->radioButton_AlarmLowOff->setChecked(true);
    ui->lineEdit_AlarmLowThr->setVisible(false);
    ui->label_AlarmLowThr->setVisible(false);
    ui->lineEdit_AlarmLowHy->setVisible(false);
    ui->label_AlarmLowHy->setVisible(false);
    connect(ui->radioButton_AlarmLowOff, SIGNAL(clicked()), this, SLOT(onRadioClickAlarmLow()));
    connect(ui->radioButton_AlarmLowOn, SIGNAL(clicked()), this, SLOT(onRadioClickAlarmLow()));

    //高温警报Radio
    btnGroupAlarmHigh = new QButtonGroup(this);
    btnGroupAlarmHigh->addButton(ui->radioButton_AlarmHighOff, 0);
    btnGroupAlarmHigh->addButton(ui->radioButton_AlarmHighOn, 1);
    ui->radioButton_AlarmHighOff->setChecked(true);
    ui->lineEdit_AlarmHighThr->setVisible(false);
    ui->label_AlarmHighThr->setVisible(false);
    ui->lineEdit_AlarmHighHy->setVisible(false);
    ui->label_AlarmHighHy->setVisible(false);
    connect(ui->radioButton_AlarmHighOff, SIGNAL(clicked()), this, SLOT(onRadioClickAlarmHigh()));
    connect(ui->radioButton_AlarmHighOn, SIGNAL(clicked()), this, SLOT(onRadioClickAlarmHigh()));

    connect(this, SIGNAL(HistoryChange(qint8)), SLOT(HistoryDisplay(qint8)));

    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date_y = current_date_time.toString("yy");
    QString current_date_m = current_date_time.toString("MM");
    QString current_date_d = current_date_time.toString("dd");
    current_date_y.append(current_date_m);
    current_date_y.append(current_date_d);
    ui->lineEdit_ID1->setText(current_date_y);
    ui->lineEdit_ID2->setText("1");

    my_serialport= new QSerialPort();
    ui->label_ComStatus->setText("COM  Close");
    setWindowIcon(QIcon(":/config_set.ico"));

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "Name        : " << info.portName();
        qDebug() << "Description : " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();

        QSerialPort serial;
        serial.setPort(info);
        if (serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox_ComName->addItem(info.portName());
            serial.close();
        }
    }
}



/**
 * Calculating CRC-16 in 'C'
 * @para addr, start of data
 * @para num, length of data
 * @para crc, incoming CRC
 */
qint16 Widget::crc16(char *addr, int num, quint16 crc)
{
    int i;
    for (; num > 0; num--)              /* Step through bytes in memory */
    {
        crc = crc ^ (*addr++ << 8);     /* Fetch byte from memory, XOR into CRC top byte*/
        for (i = 0; i < 8; i++)             /* Prepare to rotate 8 bits */
        {
            if (crc & 0x8000)            /* b15 is set... */
                crc = (crc << 1) ^ POLY;    /* rotate and XOR with polynomic */
            else                          /* b15 is clear... */
                crc <<= 1;                  /* just rotate */
        }                             /* Loop for 8 bits */
        crc &= 0xFFFF;                  /* Ensure CRC remains 16-bit value */
    }                               /* Loop until num=0 */
    return(crc);                    /* Return updated CRC */
}


Widget::~Widget()
{
    delete ui;
    if(my_serialport->isOpen())
        my_serialport->close();
    delete my_serialport;
}

/**
* 打开串口
*/
void Widget::on_pushButton_OpenCom_clicked()
{

    QString ComStatus;
    if(!my_serialport->isOpen()){
        my_serialport->setPortName(ui->comboBox_ComName->currentText());
        if(my_serialport->open(QIODevice::ReadWrite)){
            my_serialport->setBaudRate(FixBaud);
            my_serialport->setDataBits(QSerialPort::Data8);
            my_serialport->setParity(QSerialPort::NoParity);
            my_serialport->setStopBits(QSerialPort::OneStop);
            my_serialport->setFlowControl(QSerialPort::NoFlowControl);
            ui->pushButton_OpenCom->setText(QStringLiteral("关闭串口"));
            ComStatus.append(ui->comboBox_ComName->currentText());
            ComStatus.append("  Open");
            ui->label_ComStatus->setText(ComStatus);
        }
    }
    else if(my_serialport->isOpen()){
        my_serialport->close();
        ui->pushButton_OpenCom->setText(QStringLiteral("打开串口"));
        ComStatus.append("COM  Close");
        ui->label_ComStatus->setText(ComStatus);
    }
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(handleTimer()));
    connect(my_serialport, SIGNAL(readyRead()), SLOT(handleReadyRead()));
    connect(my_serialport, SIGNAL(bytesWritten(qint64)), SLOT(handlebytesWritten()));
    timer->start(500);
}

/**
* 定时器函数，用于定时处理串口数据
*/
void Widget::handleTimer()
{
    quint16 i;
    quint8 k;
    quint8 Function,ADD,Data_H,Data_L,segmengt;
    double data_double;
    if(0==(m_readData.size()%10)){//当收到的数据是整数帧时才处理（一帧10bytes）
        for(i=0;i<m_readData.size();i=i+10){
            Function = m_readData.at(i+4);
            ADD = m_readData.at(i+5);
            Data_H = m_readData.at(i+6);
            Data_L = m_readData.at(i+7);
            //上位机的请求是查询
            if(Function==Function_Inquire){
                data_double = ((Data_H<<8)+Data_L);
                if((ADD>=ADD_LineRealStart) && (ADD<=ADD_LineRealEnd)){
                    LineEdit_real[ADD-ADD_LineRealStart]->setText(QString::number(data_double/10.0,10,1));
                }
                else if((ADD>=ADD_LineLineStart) && (ADD<=ADD_LineLineEnd)){
                    LineEdit_line[ADD-ADD_LineLineStart]->setText(QString::number(data_double/10.0,10,1));
                }
                else if(ADD==ADD_SegmentNum){
                    segmengt = Data_L;
                    LineEdit_lineNum->setText(QString::number(segmengt));

                    k=segmengt;
                    while(k<30){
                        LineEdit_real[k]->setText("0");
                        LineEdit_line[k]->setText("0");
                        k++;
                    }
                }
                else if(ADD==ADD_ADcaliDLow){
                    if(data_double>=32768)
                        data_double = data_double - 65536;
                    ui->label_ADcaliDLow->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_ADcaliDHigh){
                    if(data_double>=32768)
                        data_double = data_double - 65536;
                    ui->label_ADcaliDHigh->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_ADcaliVLow){
                    ui->lineEdit_ADcaliVLow->setText(QString::number(data_double/10,10,1));
                }
                else if(ADD==ADD_ADcaliVHigh){
                    ui->lineEdit_ADcaliVHigh->setText(QString::number(data_double/10,10,1));
                }
                else if(ADD==ADD_DeviceID1){
                    quint16 data_uint = data_double;
                    quint8 year,month,day;
                    quint32 ID1,temp;
                    temp = ui->lineEdit_ID1->text().toLong();
                    day = temp%100;
                    year = (data_uint>>8)&0xff;
                    month = data_uint&0xff;
                    ID1 = year*10000 + month*100 + day;
                    ui->lineEdit_ID1->setText(QString::number(ID1,10,0));
                }
                else if(ADD==ADD_DeviceID2){
                    quint16 data_uint = data_double;
                    quint8 day,number,ID2;
                    quint32 ID1,temp;
                    temp = ui->lineEdit_ID1->text().toLong();
                    day = (data_uint>>8)&0xff;
                    number = data_uint&0xff;
                    ID1 = temp/100*100 + day;
                    ID2 = number;
                    ui->lineEdit_ID1->setText(QString::number(ID1,10,0));
                    ui->lineEdit_ID2->setText(QString::number(ID2,10,0));
                }
                else if(ADD==ADD_TRangeLow){
                    ui->lineEdit_TRangeLow->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_TRangeHigh){
                    ui->lineEdit_TRangeHigh->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_VcaliDLow){
                    ui->label_VcaliDLow->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_VcaliDHigh){
                    ui->label_VcaliDHigh->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_VcaliVLow){
                    ui->lineEdit_VcaliVLow->setText(QString::number(data_double/10,10,1));
                }
                else if(ADD==ADD_VcaliVHigh){
                    ui->lineEdit_VcaliVHigh->setText(QString::number(data_double/10,10,1));
                }
                else if(ADD==ADD_VcaliD){
                    ui->lineEdit_VcaliD->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_EMISFull){
                    ui->lineEdit_EMISFull->setText(QString::number(data_double/10,10,1));
                }
                else if(ADD==ADD_EMIS){
                    ui->lineEdit_EMIS->setText(QString::number(data_double,10,1));
                }
                else if(ADD==ADD_PeakOn){
                    if(data_double==1){
                        ui->radioButton_PeakOff->setChecked(false);
                        ui->radioButton_PeakOn->setChecked(true);
                        ui->lineEdit_PeakThr->setVisible(true);
                        ui->label_PeakThr->setVisible(true);
                        ui->lineEdit_HoldTime->setVisible(true);
                        ui->label_HoldTime->setVisible(true);
                        ui->lineEdit_DeRate->setVisible(true);
                        ui->label_DeRate->setVisible(true);
                    }
                    else{
                        ui->radioButton_PeakOn->setChecked(false);
                        ui->radioButton_PeakOff->setChecked(true);
                        ui->lineEdit_PeakThr->setVisible(false);
                        ui->label_PeakThr->setVisible(false);
                        ui->lineEdit_HoldTime->setVisible(false);
                        ui->label_HoldTime->setVisible(false);
                        ui->lineEdit_DeRate->setVisible(false);
                        ui->label_DeRate->setVisible(false);
                    }
                }
                else if(ADD==ADD_PeakThr){
                    ui->lineEdit_PeakThr->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_HoldTime){
                    ui->lineEdit_HoldTime->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_DeRate){
                    ui->lineEdit_DeRate->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_AlarmLowOn){
                    if(data_double==1){
                        ui->radioButton_AlarmLowOff->setChecked(false);
                        ui->radioButton_AlarmLowOn->setChecked(true);

                        ui->lineEdit_AlarmLowThr->setVisible(true);
                        ui->label_AlarmLowThr->setVisible(true);
                        ui->lineEdit_AlarmLowHy->setVisible(true);
                        ui->label_AlarmLowHy->setVisible(true);
                    }
                    else{
                        ui->radioButton_AlarmLowOn->setChecked(false);
                        ui->radioButton_AlarmLowOff->setChecked(true);

                        ui->lineEdit_AlarmLowThr->setVisible(false);
                        ui->label_AlarmLowThr->setVisible(false);
                        ui->lineEdit_AlarmLowHy->setVisible(false);
                        ui->label_AlarmLowHy->setVisible(false);
                    }
                }
                else if(ADD==ADD_AlarmLowThr){
                    ui->lineEdit_AlarmLowThr->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_AlarmLowHy){
                    ui->lineEdit_AlarmLowHy->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_AlarmHighOn){
                    if(data_double==1){
                        ui->radioButton_AlarmHighOff->setChecked(false);
                        ui->radioButton_AlarmHighOn->setChecked(true);

                        ui->lineEdit_AlarmHighThr->setVisible(true);
                        ui->label_AlarmHighThr->setVisible(true);
                        ui->lineEdit_AlarmHighHy->setVisible(true);
                        ui->label_AlarmHighHy->setVisible(true);
                    }
                    else{
                        ui->radioButton_AlarmHighOn->setChecked(false);
                        ui->radioButton_AlarmHighOff->setChecked(true);

                        ui->lineEdit_AlarmHighThr->setVisible(false);
                        ui->label_AlarmHighThr->setVisible(false);
                        ui->lineEdit_AlarmHighHy->setVisible(false);
                        ui->label_AlarmHighHy->setVisible(false);
                    }
                }
                else if(ADD==ADD_AlarmHighThr){
                    ui->lineEdit_AlarmHighThr->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_AlarmHighHy){
                    ui->lineEdit_AlarmHighHy->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_ResTime){
                    ui->lineEdit_ResTime->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_AcaliOn){
                    quint16 data_uint = data_double;
                    if(data_uint==1)
                        ui->label_AcaliOn->setText(QStringLiteral("电流校验模式！"));
                    else
                        ui->label_AcaliOn->setText(QStringLiteral("非电流校验模式！"));
                }
                else if(ADD==ADD_AcaliDLow){
                    ui->label_AcaliDLow->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_AcaliDHigh){
                    ui->label_AcaliDHigh->setText(QString::number(data_double,10,0));
                }
                else if(ADD==ADD_AcaliD){
                    ui->lineEdit_AcaliD->setText(QString::number(data_double,10,0));
                }
            }
        }
        m_readData.clear();
    }

}


/**
 * 获取数据帧
 * @para func, 功能码:3:查询；6：修改
 * @para add, 查询或修改寄存器的地址
 * @para data, 需要发送的数据
 * @para data_frame, 组好的数据帧
 */
void Widget::GetDataFrame(char* ID, quint8 func,quint8 add,quint16 data, char* data_frame)
{
    qint16 crc_code;

    //实际电压数据帧
    *(data_frame+0) = *(ID+0);
    *(data_frame+1) = *(ID+1);
    *(data_frame+2) = *(ID+2);
    *(data_frame+3) = *(ID+3);
    *(data_frame+4) = func;
    *(data_frame+5) = add;
    *(data_frame+6) = data>>8;
    *(data_frame+7) = data&0xff;

    //计算CRC
    crc_code = crc16(data_frame, 8, 0xffff);
    *(data_frame+8) = crc_code>>8;
    *(data_frame+9) = crc_code&0xff;

    crc_code = crc16(data_frame, 10, 0xffff);
}


/**
 * Text Edit自动滚屏
 * @para TextEdit, Text Edit
 * @para string, 需要添加的文字
 */
void Widget::AutoScroll(QTextEdit *TextEdit, QString &string)
{
    TextEdit->append(string);
    QTextCursor cursor = TextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    TextEdit->setTextCursor(cursor);
}

/**
* 线性化数据写入
*/
void Widget::on_pushButton_LineWrite_clicked()
{
    quint8 Segment_Num;
    quint8 i;
    quint16 real, line;
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        Segment_Num = ui->lineEdit_lineNum->text().toInt();
        if((Segment_Num < Segment_Low)||(Segment_Num > Segment_High))
            emit HistoryChange(SIGNAL_segment_Error);
        else{
            for(i=0;i<Segment_Num;i++){
                //实际电压
                real = LineEdit_real[i]->text().toDouble()*10;
                GetDataFrame(&SuperID[0], Function_Modify,i+ADD_LineRealStart,real,DataPtr);
                my_serialport->write(DataPtr, 10);
                //线性电压
                line = LineEdit_line[i]->text().toDouble()*10;
                GetDataFrame(&SuperID[0], Function_Modify,i+ADD_LineLineStart,line,DataPtr);
                my_serialport->write(DataPtr, 10);
            }
            //线性化段数
            GetDataFrame(&SuperID[0], Function_Modify,ADD_SegmentNum,Segment_Num,DataPtr);
            my_serialport->write(DataPtr, 10);

            //线性化发送完毕，通知MCU更新FLASH
            GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
            my_serialport->write(DataPtr, 10);

            emit HistoryChange(SIGNAL_LineWrite);
        }
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}


/**
* 线性化数据读取
*/
void Widget::on_pushButton_LineRead_clicked()
{
    quint8 k;
    for(k=0;k<30;k++){
        //对话框初始化
        LineEdit_real[k]->clear();
        LineEdit_line[k]->clear();
    }
    LineEdit_lineNum->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_LineRead,LineRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_LineRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 搜索串口
*/
void Widget::on_pushButton_SearchCom_clicked()
{
    ui->comboBox_ComName->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "Name        : " << info.portName();
        qDebug() << "Description : " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();

        QSerialPort serial;
        serial.setPort(info);
        if (serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox_ComName->addItem(info.portName());
            serial.close();
        }
    }
}

/**
* 发送信号，接收信号
*/
void Widget::handlebytesWritten()
{
    ui->label_TXcnt->setNum(++TXcnt);
}
void Widget::handleReadyRead()
{
    ui->label_RXcnt->setNum(++RXcnt);
    m_readData.append(my_serialport->readAll());
    AutoScroll(TextEdit_History, QString::number(m_readData.length()));

    //因为一帧的长度是10byte，不会出现奇数byte的情况
    if(m_readData.length()%2==1)
        m_readData.clear();   
}

/**
* 串口计数器清零
*/
void Widget::on_pushButton_RXTXclear_clicked()
{
    RXcnt = 0;
    TXcnt = 0;
    ui->label_RXcnt->setNum(RXcnt);
    ui->label_TXcnt->setNum(TXcnt);
}

/**
* 线性化数据填入测试数据
*/
void Widget::on_pushButton_LineTest_clicked()
{
    //测试时，对话框初始化
    LineEdit_real[0]->setText("0.1");
    LineEdit_real[1]->setText("0.2");
    LineEdit_real[2]->setText("0.3");
    LineEdit_real[3]->setText("0.4");
    LineEdit_real[4]->setText("0.5");
    LineEdit_real[5]->setText("0.6");
    LineEdit_real[6]->setText("0.7");
    LineEdit_real[7]->setText("0.8");
    LineEdit_real[8]->setText("0.9");
    LineEdit_real[9]->setText("1");
    LineEdit_real[10]->setText("1.1");
    LineEdit_real[11]->setText("1.2");
    LineEdit_real[12]->setText("1.3");
    LineEdit_real[13]->setText("1.4");
    LineEdit_real[14]->setText("1.5");
    LineEdit_real[15]->setText("1.6");
    LineEdit_real[16]->setText("1.7");
    LineEdit_real[17]->setText("1.8");
    LineEdit_real[18]->setText("1.9");
    LineEdit_real[19]->setText("2");
    LineEdit_real[20]->setText("2.1");
    LineEdit_real[21]->setText("2.2");
    LineEdit_real[22]->setText("2.3");
    LineEdit_real[23]->setText("2.4");
    LineEdit_real[24]->setText("2.5");
    LineEdit_real[25]->setText("2.6");
    LineEdit_real[26]->setText("2.7");
    LineEdit_real[27]->setText("2.8");
    LineEdit_real[28]->setText("2.9");
    LineEdit_real[29]->setText("3");

    LineEdit_line[0]->setText("3.1");
    LineEdit_line[1]->setText("3.2");
    LineEdit_line[2]->setText("3.3");
    LineEdit_line[3]->setText("3.4");
    LineEdit_line[4]->setText("3.5");
    LineEdit_line[5]->setText("3.6");
    LineEdit_line[6]->setText("3.7");
    LineEdit_line[7]->setText("3.8");
    LineEdit_line[8]->setText("3.9");
    LineEdit_line[9]->setText("4");
    LineEdit_line[10]->setText("4.1");
    LineEdit_line[11]->setText("4.2");
    LineEdit_line[12]->setText("4.3");
    LineEdit_line[13]->setText("4.4");
    LineEdit_line[14]->setText("4.5");
    LineEdit_line[15]->setText("4.6");
    LineEdit_line[16]->setText("4.7");
    LineEdit_line[17]->setText("4.8");
    LineEdit_line[18]->setText("4.9");
    LineEdit_line[19]->setText("5");
    LineEdit_line[20]->setText("5.1");
    LineEdit_line[21]->setText("5.2");
    LineEdit_line[22]->setText("5.3");
    LineEdit_line[23]->setText("5.4");
    LineEdit_line[24]->setText("5.5");
    LineEdit_line[25]->setText("5.6");
    LineEdit_line[26]->setText("5.7");
    LineEdit_line[27]->setText("5.8");
    LineEdit_line[28]->setText("5.9");
    LineEdit_line[29]->setText("6");

    LineEdit_lineNum->setText("30");
}


/**
* 清空线性化数据
*/
void Widget::on_pushButton_LineClear_clicked()
{
    quint8 k;
    for(k=0;k<30;k++){
        //对话框初始化
        LineEdit_real[k]->clear();
        LineEdit_line[k]->clear();
    }
    LineEdit_lineNum->clear();
}

/**
* AD低校验写入
*/
void Widget::on_pushButton_ADcaliLow_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    quint16 ADcaliVLow;
    if(my_serialport->isOpen()){
        ADcaliVLow = ui->lineEdit_ADcaliVLow->text().toDouble()*10;
        GetDataFrame(&SuperID[0], Function_Modify, ADD_ADcaliVLow, ADcaliVLow, DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_ADcaliLow);
    }
    else
        emit HistoryChange(SIGNAL_Open_Error);
}

/**
* AD高校验写入
*/
void Widget::on_pushButton_ADcaliHigh_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    quint16 ADcaliVHigh;
    if(my_serialport->isOpen()){
        ADcaliVHigh = ui->lineEdit_ADcaliVHigh->text().toDouble()*10;
        GetDataFrame(&SuperID[0], Function_Modify, ADD_ADcaliVHigh, ADcaliVHigh, DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_ADcaliHigh);
    }
    else
        emit HistoryChange(SIGNAL_Open_Error);
}


/**
* 记录栏更新记录
* @para index, 更新记录的类型
*/
void Widget::HistoryDisplay(qint8 index)
{
    switch(index){
    case SIGNAL_IDRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取出厂编号！"));
        break;

    case SIGNAL_IDWrite:
        AutoScroll(TextEdit_History, QStringLiteral("写入出厂编号！"));
        break;

    case SIGNAL_LineRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取线性化数据！"));
        break;

    case SIGNAL_LineWrite:
        AutoScroll(TextEdit_History, QStringLiteral("写入线性化数据！"));
        break;

    case SIGNAL_segment_Error:
        AutoScroll(TextEdit_History, QStringLiteral("请输入1-30之间的点数！"));
        break;

    case SIGNAL_ADcaliHigh:
        AutoScroll(TextEdit_History, QStringLiteral("AD高校验！"));
        break;

    case SIGNAL_ADcaliLow:
        AutoScroll(TextEdit_History, QStringLiteral("AD低校验！"));
        break;

    case SIGNAL_ADcaliRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取AD校验数据！"));
        break;

    case SIGNAL_TWrite:
        AutoScroll(TextEdit_History, QStringLiteral("写入温度范围！"));
        break;

    case SIGNAL_TRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取温度范围！"));
        break;

    case SIGNAL_Open_Error:
        AutoScroll(TextEdit_History, QStringLiteral("串口未开启！"));
        break;

    case SIGNAL_VcaliD:
        AutoScroll(TextEdit_History, QStringLiteral("DA输出电压！"));
        break;

    case SIGNAL_VcaliVLow:
        AutoScroll(TextEdit_History, QStringLiteral("设置电压下限！"));
        break;

    case SIGNAL_VcaliVHigh:
        AutoScroll(TextEdit_History, QStringLiteral("设置电压上限！"));
        break;

    case SIGNAL_VcaliRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取电压校验！"));
        break;

    case SIGNAL_EMISWrite:
        AutoScroll(TextEdit_History, QStringLiteral("写入发射率！"));
        break;

    case SIGNAL_EMISRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取发射率！"));
        break;

    case SIGNAL_PeakWrite:
        AutoScroll(TextEdit_History, QStringLiteral("写入峰值保持设置！"));
        break;

    case SIGNAL_PeakRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取峰值保持设置！"));
        break;

    case SIGNAL_AlarmLowWrite:
        AutoScroll(TextEdit_History, QStringLiteral("写入低温报警设置！"));
        break;

    case SIGNAL_AlarmLowRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取低温报警设置！"));
        break;

    case SIGNAL_AlarmHighWrite:
        AutoScroll(TextEdit_History, QStringLiteral("写入高温报警设置！"));
        break;

    case SIGNAL_AlarmHighRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取高温报警设置！"));
        break;

    case SIGNAL_ResTimeWrite:
        AutoScroll(TextEdit_History, QStringLiteral("写入响应时间！"));
        break;

    case SIGNAL_ResTimeRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取响应时间！"));
        break;

    case SIGNAL_AcaliOn:
        AutoScroll(TextEdit_History, QStringLiteral("进入电流校验！"));
        ui->label_AcaliOn->setText(QStringLiteral("电流校验模式"));
        break;

    case SIGNAL_AcaliOff:
        AutoScroll(TextEdit_History, QStringLiteral("退出电流校验！"));
        ui->label_AcaliOn->setText(QStringLiteral("非电流校验模式"));
        break;

    case SIGNAL_AcaliDLow:
        AutoScroll(TextEdit_History, QStringLiteral("设置电流校验下限！"));
        break;

    case SIGNAL_AcaliDHigh:
        AutoScroll(TextEdit_History, QStringLiteral("设置电流校验上限！"));
        break;

    case SIGNAL_AcaliOut:
        AutoScroll(TextEdit_History, QStringLiteral("电流校验D值输出！"));
        break;

    case SIGNAL_AcaliRead:
        AutoScroll(TextEdit_History, QStringLiteral("读取电流校验！"));
        break;

    case SIGNAL_EMIS_Error:
        AutoScroll(TextEdit_History, QStringLiteral("百分比应该在0~100%之间！"));
        break;

    case SIGNAL_Excel_Format:
        AutoScroll(TextEdit_History, QStringLiteral("Excel格式错误！"));
        break;

    case SIGNAL_Excel_NoFile:
        AutoScroll(TextEdit_History, QStringLiteral("Excel文件不存在！"));
        break;

    default:break;
    }
}

/**
* AD校验清空
*/
//void Widget::on_pushButton_ADcaliClear_clicked()
//{
//    ui->label_ADcaliDHigh->clear();
//    ui->label_ADcaliDLow->clear();
//    ui->lineEdit_ADcaliVHigh->clear();
//    ui->lineEdit_ADcaliVLow->clear();
//}

/**
* AD校验数据读取
*/
void Widget::on_pushButton_ADcaliRead_clicked()
{
    ui->label_ADcaliDHigh->clear();
    ui->label_ADcaliDLow->clear();
    ui->lineEdit_ADcaliVHigh->clear();
    ui->lineEdit_ADcaliVLow->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_ADcaliRead,ADcaliRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_ADcaliRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* ID数据读取
*/
void Widget::on_pushButton_IDRead_clicked()
{
    ui->lineEdit_ID1->clear();
    ui->lineEdit_ID2->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_DeviceIDRead,IDRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_IDRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* ID数据写入
*/
void Widget::on_pushButton_IDWrite_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    quint32 ID1,ID2;
    quint8 year,month,day,number;
    quint16 data1,data2;
    if(my_serialport->isOpen()){
        ID1 = ui->lineEdit_ID1->text().toLong();
        ID2 = ui->lineEdit_ID2->text().toLong();
        year = ID1/10000;
        month = ID1/100%100;
        day = ID1%100;
        number = ID2;
        data1 = (year<<8) + month;
        GetDataFrame(&SuperID[0], Function_Modify, ADD_DeviceID1, data1, DataPtr);
        my_serialport->write(DataPtr, 10);

        data2 = (day<<8) + number;
        GetDataFrame(&SuperID[0], Function_Modify, ADD_DeviceID2, data2, DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_IDWrite);
    }
    else
        emit HistoryChange(SIGNAL_Open_Error);
}

/**
* 获取当前日期填入ID
*/
void Widget::on_pushButton_IDToday_clicked()
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date_y = current_date_time.toString("yy");
    QString current_date_m = current_date_time.toString("MM");
    QString current_date_d = current_date_time.toString("dd");
    current_date_y.append(current_date_m);
    current_date_y.append(current_date_d);
    ui->lineEdit_ID1->setText(current_date_y);
    ui->lineEdit_ID2->setText("1");
}

/**
* 写入温度、电压范围
*/
void Widget::on_pushButton_TRangeWrite_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    quint16 TRangeLow, TRangeHigh;
    if(my_serialport->isOpen()){
        TRangeHigh = ui->lineEdit_TRangeHigh->text().toDouble();
        TRangeLow = ui->lineEdit_TRangeLow->text().toDouble();
        GetDataFrame(&SuperID[0], Function_Modify, ADD_TRangeLow, TRangeLow, DataPtr);
        my_serialport->write(DataPtr, 10);

        GetDataFrame(&SuperID[0], Function_Modify, ADD_TRangeHigh, TRangeHigh, DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_TWrite);
    }
    else
        emit HistoryChange(SIGNAL_Open_Error);
}

/**
* 读取温度、电压范围
*/
void Widget::on_pushButton_TRangeRead_clicked()
{
    ui->lineEdit_TRangeLow->clear();
    ui->lineEdit_TRangeHigh->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_TRangeRead,TRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_TRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}


/**
* 仪器参数恢复默认值
*/
void Widget::on_pushButton_init_clicked()
{
    BasicExcel e;
    BasicExcelWorksheet* sheet1;
    double data_double;
    QString runPath = QCoreApplication::applicationDirPath();
    runPath.append("/data.xls");
    std::string str = runPath.toStdString();
    const char* file = str.c_str();
    //    e.Load(file);
    e.Load("E:/QTproject/data.xls");
    sheet1 = e.GetWorksheet("Sheet1");
    if(sheet1)
    {
        size_t maxRows = sheet1->GetTotalRows();
        size_t maxCols = sheet1->GetTotalCols();
        if(maxCols<2){
            emit HistoryChange(SIGNAL_Excel_Error);
            return;
        }
        for(size_t r=1; r<maxRows; r++){
            for(size_t c =1; c<=2; c++){
                BasicExcelCell* cell = sheet1->Cell(r,c);
                switch (cell->Type()){//选择输出的格式
                    case BasicExcelCell::UNDEFINED:
                        qDebug()<<(" ");
                    break;
                    case BasicExcelCell::INT:
                        qDebug()<<("%10d", cell->GetInteger());
                    break;
                    case BasicExcelCell::DOUBLE:
                        data_double = cell->GetDouble();
                        qDebug()<<("%10.6lf", cell->GetDouble());
                        if(c==1)
                            LineEdit_real[r-1]->setText(QString::number(data_double,10,1));
                        else if(c==2)
                            LineEdit_line[r-1]->setText(QString::number(data_double,10,1));
                    break;
                    case BasicExcelCell::STRING:
                        qDebug()<<("%10s", cell->GetString());
                    break;
                    case BasicExcelCell::WSTRING:
                        qDebug()<<(L"%10s", cell->GetWString());
                    break;
                }
            }
            qDebug() << endl;
        }
    }
}

/**
* 电压校验：电压输出
*/
void Widget::on_pushButton_VcaliVout_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    double VcaliD,EMIS;
    if(my_serialport->isOpen()){
        VcaliD = ui->lineEdit_VcaliD->text().toDouble();
        GetDataFrame(&SuperID[0], Function_Modify,ADD_VcaliD,VcaliD,DataPtr);
        my_serialport->write(DataPtr, 10);

        EMIS = 200;//使EMIS大于100%，这时按D值进行输出
        GetDataFrame(&SuperID[0], Function_Modify,ADD_EMIS,EMIS,DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_VcaliD);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 电压校验：设置电压下限
*/
void Widget::on_pushButton_VcaliLow_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    double VcaliVLow,VcaliDLow;
    if(my_serialport->isOpen()){
        VcaliVLow = ui->lineEdit_VcaliVLow->text().toDouble()*10;
        GetDataFrame(&SuperID[0], Function_Modify,ADD_VcaliVLow,VcaliVLow,DataPtr);
        my_serialport->write(DataPtr, 10);

        VcaliDLow = ui->lineEdit_VcaliD->text().toDouble();
        GetDataFrame(&SuperID[0], Function_Modify,ADD_VcaliDLow,VcaliDLow,DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_VcaliVLow);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 电压校验：设置电压上限
*/
void Widget::on_pushButton_VcaliHigh_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    double VcaliVHigh, VcaliDHigh;
    if(my_serialport->isOpen()){
        VcaliVHigh = ui->lineEdit_VcaliVHigh->text().toDouble()*10;
        GetDataFrame(&SuperID[0], Function_Modify,ADD_VcaliVHigh,VcaliVHigh,DataPtr);
        my_serialport->write(DataPtr, 10);

        VcaliDHigh = ui->lineEdit_VcaliD->text().toDouble();
        GetDataFrame(&SuperID[0], Function_Modify,ADD_VcaliDHigh,VcaliDHigh,DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_VcaliVHigh);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}


/**
* 电压校验：读取
*/
void Widget::on_pushButton_VcaliRead_clicked()
{
    ui->lineEdit_VcaliD->clear();
    ui->lineEdit_VcaliVLow->clear();
    ui->lineEdit_VcaliVHigh->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_VcaliRead,VcaliRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_VcaliRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 写入发射率
*/
void Widget::on_pushButton_EMISWrite_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    double EMISFull,EMIS;
    if(my_serialport->isOpen()){
        if((ui->lineEdit_EMIS->text().toDouble()>100) || (ui->lineEdit_EMIS->text().toDouble()<0))
            emit HistoryChange(SIGNAL_EMIS_Error);
        else{
            EMISFull = ui->lineEdit_EMISFull->text().toDouble()*10;
            GetDataFrame(&SuperID[0], Function_Modify,ADD_EMISFull,EMISFull,DataPtr);
            my_serialport->write(DataPtr, 10);

            EMIS = ui->lineEdit_EMIS->text().toDouble();
            GetDataFrame(&SuperID[0], Function_Modify,ADD_EMIS,EMIS,DataPtr);
            my_serialport->write(DataPtr, 10);

            //数据发送完毕，通知MCU更新FLASH
            GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
            my_serialport->write(DataPtr, 10);

            emit HistoryChange(SIGNAL_EMISWrite);
        }
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 读取发射率
*/
void Widget::on_pushButton_EMISRead_clicked()
{
    ui->lineEdit_EMISFull->clear();
    ui->lineEdit_EMIS->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_EMISRead,EMISRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_EMISRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 写入峰值
*/
void Widget::on_pushButton_PeakWrite_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    quint16 PeakOn,PeakThr,HoldTime,DeRate;

    switch(btnGroupPeak->checkedId())
    {
    case 0://Peak Off
        if(my_serialport->isOpen()){
            PeakOn = 0;
            GetDataFrame(&SuperID[0], Function_Modify,ADD_PeakOn,PeakOn,DataPtr);
            my_serialport->write(DataPtr, 10);

            //数据发送完毕，通知MCU更新FLASH
            GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
            my_serialport->write(DataPtr, 10);

            emit HistoryChange(SIGNAL_PeakWrite);
        }
        else{
            emit HistoryChange(SIGNAL_Open_Error);
        }
        break;

    case 1://Peak On
        if(my_serialport->isOpen()){
            PeakOn = 1;
            GetDataFrame(&SuperID[0], Function_Modify,ADD_PeakOn,PeakOn,DataPtr);
            my_serialport->write(DataPtr, 10);

            PeakThr = ui->lineEdit_PeakThr->text().toDouble();
            GetDataFrame(&SuperID[0], Function_Modify,ADD_PeakThr,PeakThr,DataPtr);
            my_serialport->write(DataPtr, 10);

            HoldTime = ui->lineEdit_HoldTime->text().toDouble();
            GetDataFrame(&SuperID[0], Function_Modify,ADD_HoldTime,HoldTime,DataPtr);
            my_serialport->write(DataPtr, 10);

            DeRate = ui->lineEdit_DeRate->text().toDouble();
            GetDataFrame(&SuperID[0], Function_Modify,ADD_DeRate,DeRate,DataPtr);
            my_serialport->write(DataPtr, 10);

            //数据发送完毕，通知MCU更新FLASH
            GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
            my_serialport->write(DataPtr, 10);

            emit HistoryChange(SIGNAL_PeakWrite);
        }
        else{
            emit HistoryChange(SIGNAL_Open_Error);
        }
        break;
    }
}

/**
* 峰值Radio响应
*/
void Widget::onRadioClickPeak()
{
    switch(btnGroupPeak->checkedId())
    {
    case 0://Peak Off
        ui->lineEdit_PeakThr->setVisible(false);
        ui->label_PeakThr->setVisible(false);
        ui->lineEdit_HoldTime->setVisible(false);
        ui->label_HoldTime->setVisible(false);
        ui->lineEdit_DeRate->setVisible(false);
        ui->label_DeRate->setVisible(false);
        break;

    case 1://Peak On
        ui->lineEdit_PeakThr->setVisible(true);
        ui->label_PeakThr->setVisible(true);
        ui->lineEdit_HoldTime->setVisible(true);
        ui->label_HoldTime->setVisible(true);
        ui->lineEdit_DeRate->setVisible(true);
        ui->label_DeRate->setVisible(true);
        break;
    }
}


/**
* 读取峰值
*/
void Widget::on_pushButton_PeakRead_clicked()
{
    ui->lineEdit_PeakThr->clear();
    ui->lineEdit_HoldTime->clear();
    ui->lineEdit_DeRate->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_PeakRead,PeakRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_PeakRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}


/**
* 低温报警Radio响应
*/
void Widget::onRadioClickAlarmLow()
{
    switch(btnGroupAlarmLow->checkedId())
    {
    case 0://AlarmLow Off
        ui->lineEdit_AlarmLowThr->setVisible(false);
        ui->label_AlarmLowThr->setVisible(false);
        ui->lineEdit_AlarmLowHy->setVisible(false);
        ui->label_AlarmLowHy->setVisible(false);
        break;

    case 1://AlarmLow On
        ui->lineEdit_AlarmLowThr->setVisible(true);
        ui->label_AlarmLowThr->setVisible(true);
        ui->lineEdit_AlarmLowHy->setVisible(true);
        ui->label_AlarmLowHy->setVisible(true);
        break;
    }
}

/**
* 写入低温报警
*/
void Widget::on_pushButton_AlarmLowWrite_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    quint16 AlarmLowOn,AlarmLowThr,AlarmLowHy;

    switch(btnGroupAlarmLow->checkedId())
    {
    case 0://AlarmLow Off
        if(my_serialport->isOpen()){
            AlarmLowOn = 0;
            GetDataFrame(&SuperID[0], Function_Modify,ADD_AlarmLowOn,AlarmLowOn,DataPtr);
            my_serialport->write(DataPtr, 10);

            //数据发送完毕，通知MCU更新FLASH
            GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
            my_serialport->write(DataPtr, 10);

            emit HistoryChange(SIGNAL_AlarmLowWrite);
        }
        else{
            emit HistoryChange(SIGNAL_Open_Error);
        }
        break;

    case 1://AlarmLow On
        if(my_serialport->isOpen()){
            AlarmLowOn = 1;
            GetDataFrame(&SuperID[0], Function_Modify,ADD_AlarmLowOn,AlarmLowOn,DataPtr);
            my_serialport->write(DataPtr, 10);

            AlarmLowThr = ui->lineEdit_AlarmLowThr->text().toDouble();
            GetDataFrame(&SuperID[0], Function_Modify,ADD_AlarmLowThr,AlarmLowThr,DataPtr);
            my_serialport->write(DataPtr, 10);

            AlarmLowHy = ui->lineEdit_AlarmLowHy->text().toDouble();
            GetDataFrame(&SuperID[0], Function_Modify,ADD_AlarmLowHy,AlarmLowHy,DataPtr);
            my_serialport->write(DataPtr, 10);

            //数据发送完毕，通知MCU更新FLASH
            GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
            my_serialport->write(DataPtr, 10);

            emit HistoryChange(SIGNAL_AlarmLowWrite);
        }
        else{
            emit HistoryChange(SIGNAL_Open_Error);
        }
        break;
    }
}


/**
* 读取低温报警
*/
void Widget::on_pushButton_AlarmLowRead_clicked()
{
    ui->lineEdit_AlarmLowThr->clear();
    ui->lineEdit_AlarmLowHy->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_AlarmLowRead,AlarmLowRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_AlarmLowRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 高温报警Radio响应
*/
void Widget::onRadioClickAlarmHigh()
{
    switch(btnGroupAlarmHigh->checkedId())
    {
    case 0://AlarmHigh Off
        ui->lineEdit_AlarmHighThr->setVisible(false);
        ui->label_AlarmHighThr->setVisible(false);
        ui->lineEdit_AlarmHighHy->setVisible(false);
        ui->label_AlarmHighHy->setVisible(false);
        break;

    case 1://AlarmHigh On
        ui->lineEdit_AlarmHighThr->setVisible(true);
        ui->label_AlarmHighThr->setVisible(true);
        ui->lineEdit_AlarmHighHy->setVisible(true);
        ui->label_AlarmHighHy->setVisible(true);
        break;
    }
}

/**
* 写入高温报警
*/
void Widget::on_pushButton_AlarmHighWrite_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    quint16 AlarmHighOn,AlarmHighThr,AlarmHighHy;

    switch(btnGroupAlarmHigh->checkedId())
    {
    case 0://AlarmHigh Off
        if(my_serialport->isOpen()){
            AlarmHighOn = 0;
            GetDataFrame(&SuperID[0], Function_Modify,ADD_AlarmHighOn,AlarmHighOn,DataPtr);
            my_serialport->write(DataPtr, 10);

            //数据发送完毕，通知MCU更新FLASH
            GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
            my_serialport->write(DataPtr, 10);

            emit HistoryChange(SIGNAL_AlarmHighWrite);
        }
        else{
            emit HistoryChange(SIGNAL_Open_Error);
        }
        break;

    case 1://AlarmHigh On
        if(my_serialport->isOpen()){
            AlarmHighOn = 1;
            GetDataFrame(&SuperID[0], Function_Modify,ADD_AlarmHighOn,AlarmHighOn,DataPtr);
            my_serialport->write(DataPtr, 10);

            AlarmHighThr = ui->lineEdit_AlarmHighThr->text().toDouble();
            GetDataFrame(&SuperID[0], Function_Modify,ADD_AlarmHighThr,AlarmHighThr,DataPtr);
            my_serialport->write(DataPtr, 10);

            AlarmHighHy = ui->lineEdit_AlarmHighHy->text().toDouble();
            GetDataFrame(&SuperID[0], Function_Modify,ADD_AlarmHighHy,AlarmHighHy,DataPtr);
            my_serialport->write(DataPtr, 10);

            //数据发送完毕，通知MCU更新FLASH
            GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
            my_serialport->write(DataPtr, 10);

            emit HistoryChange(SIGNAL_AlarmHighWrite);
        }
        else{
            emit HistoryChange(SIGNAL_Open_Error);
        }
        break;
    }
}

/**
* 读取高温报警
*/
void Widget::on_pushButton_AlarmHighRead_clicked()
{
    ui->lineEdit_AlarmHighThr->clear();
    ui->lineEdit_AlarmHighHy->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_AlarmHighRead,AlarmHighRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_AlarmHighRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 写入响应时间
*/
void Widget::on_pushButton_ResTimeWrite_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    double ResTime;
    if(my_serialport->isOpen()){
        ResTime = ui->lineEdit_ResTime->text().toDouble();
        GetDataFrame(&SuperID[0], Function_Modify,ADD_ResTime,ResTime,DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_ResTimeWrite);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}


/**
* 读取响应时间
*/
void Widget::on_pushButton_ResTimeRead_clicked()
{
    ui->lineEdit_ResTime->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_ResTimeRead,ResTimeRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_ResTimeRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 进入电流校验
*/
void Widget::on_pushButton_AcaliOn_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    double AcaliOn;
    if(my_serialport->isOpen()){
        AcaliOn = 1;
        GetDataFrame(&SuperID[0], Function_Modify,ADD_AcaliOn,AcaliOn,DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        ui->pushButton_AcaliVout->setEnabled(true);
        ui->pushButton_AcaliHigh->setEnabled(true);
        ui->pushButton_AcaliLow->setEnabled(true);

        emit HistoryChange(SIGNAL_AcaliOn);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 退出电流校验
*/
void Widget::on_pushButton_AcaliOff_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    double AcaliOn;
    if(my_serialport->isOpen()){
        AcaliOn = 0;
        GetDataFrame(&SuperID[0], Function_Modify,ADD_AcaliOn,AcaliOn,DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        ui->pushButton_AcaliVout->setEnabled(false);
        ui->pushButton_AcaliHigh->setEnabled(false);
        ui->pushButton_AcaliLow->setEnabled(false);
        emit HistoryChange(SIGNAL_AcaliOff);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 设置电流校验D值 DAC0 OUT1
*/
void Widget::on_pushButton_AcaliVout_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    double AcaliD;
    if(my_serialport->isOpen()){
        AcaliD = ui->lineEdit_AcaliD->text().toDouble();
        GetDataFrame(&SuperID[0], Function_Modify,ADD_AcaliD,AcaliD,DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_AcaliOut);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 设置电流校验下限D值 DAC0 OUT1
*/
void Widget::on_pushButton_AcaliLow_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    double AcaliDLow;
    if(my_serialport->isOpen()){
        AcaliDLow = ui->lineEdit_AcaliD->text().toDouble();
        GetDataFrame(&SuperID[0], Function_Modify,ADD_AcaliDLow,AcaliDLow,DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_AcaliDLow);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}

/**
* 设置电流校验上限D值 DAC0 OUT1
*/
void Widget::on_pushButton_AcaliHigh_clicked()
{
    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    double AcaliDHigh;
    if(my_serialport->isOpen()){
        AcaliDHigh = ui->lineEdit_AcaliD->text().toDouble();
        GetDataFrame(&SuperID[0], Function_Modify,ADD_AcaliDHigh,AcaliDHigh,DataPtr);
        my_serialport->write(DataPtr, 10);

        //数据发送完毕，通知MCU更新FLASH
        GetDataFrame(&SuperID[0], Function_UpdateFlash,0,0,DataPtr);
        my_serialport->write(DataPtr, 10);

        emit HistoryChange(SIGNAL_AcaliDHigh);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}


/**
* 读取电流校验 DAC0 OUT1
*/
void Widget::on_pushButton_AcaliRead_clicked()
{
    ui->lineEdit_AcaliD->clear();
    ui->label_AcaliDLow->clear();
    ui->label_AcaliDHigh->clear();
    ui->label_AcaliOn->clear();

    char Data[10];
    char* DataPtr;
    DataPtr = &Data[0];
    if(my_serialport->isOpen()){
        GetDataFrame(&SuperID[0], Function_Inquire,ADD_AcaliRead,AcaliRegNum,DataPtr);
        my_serialport->write(DataPtr, 10);
        emit HistoryChange(SIGNAL_AcaliRead);
    }
    else{
        emit HistoryChange(SIGNAL_Open_Error);
    }
}
