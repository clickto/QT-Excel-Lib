#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "ui_widget.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "BasicExcel.hpp"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void handleTimer();
    void onRadioClickPeak();
    void onRadioClickAlarmLow();
    void onRadioClickAlarmHigh();

    void on_pushButton_OpenCom_clicked();

    void handleReadyRead();
    void handlebytesWritten();
    void on_pushButton_LineWrite_clicked();
    void on_pushButton_LineRead_clicked();

    void on_pushButton_SearchCom_clicked();

    void on_pushButton_RXTXclear_clicked();

    qint16 crc16(char *addr, int num, quint16 crc);
    void GetDataFrame(char *ID, quint8 func, quint8 add, quint16 data, char* data_frame);

    void on_pushButton_LineTest_clicked();

    void on_pushButton_LineClear_clicked();

    void AutoScroll(QTextEdit *TextEdit, QString &string);

    void on_pushButton_ADcaliLow_clicked();

    void on_pushButton_ADcaliHigh_clicked();

    void HistoryDisplay(qint8 index);

//    void on_pushButton_ADcaliClear_clicked();

    void on_pushButton_ADcaliRead_clicked();

//    void on_pushButton_IDClear_clicked();

    void on_pushButton_IDRead_clicked();

//    void on_pushButton_clicked();

    void on_pushButton_IDToday_clicked();

//    void on_pushButton_TempWrite_clicked();

//    void on_pushButton_TempRead_clicked();

//    void on_pushButton_TempClear_clicked();

    void on_pushButton_init_clicked();

    void on_pushButton_VcaliVout_clicked();

    void on_pushButton_VcaliLow_clicked();

    void on_pushButton_VcaliHigh_clicked();

//    void on_pushButton_VcaliClear_clicked();

    void on_pushButton_VcaliRead_clicked();

//    void on_pushButton_Vout_clicked();

//    void on_pushButton_VoutClear_clicked();

//    void on_pushButton_VoutRead_clicked();

    void on_pushButton_PeakWrite_clicked();

//    void on_pushButton_PeakClear_clicked();

    void on_pushButton_PeakRead_clicked();

    void on_pushButton_AlarmLowWrite_clicked();

//    void on_pushButton_AlarmLowClear_clicked();

    void on_pushButton_AlarmLowRead_clicked();

    void on_pushButton_AlarmHighWrite_clicked();

//    void on_pushButton_AlarmHighClear_clicked();

    void on_pushButton_AlarmHighRead_clicked();

    void on_pushButton_ResTimeWrite_clicked();

//    void on_pushButton_ResTimeClear_clicked();

    void on_pushButton_ResTimeRead_clicked();

    void on_pushButton_AcaliOn_clicked();

    void on_pushButton_AcaliOff_clicked();

    void on_pushButton_AcaliVout_clicked();

    void on_pushButton_AcaliLow_clicked();

    void on_pushButton_AcaliHigh_clicked();

//    void on_pushButton_AcaliClear_clicked();

    void on_pushButton_AcaliRead_clicked();

//    void on_pushButton_VcaliD_clicked();

    void on_pushButton_EMISRead_clicked();

    void on_pushButton_EMISWrite_clicked();

    void on_pushButton_IDWrite_clicked();

    void on_pushButton_TRangeWrite_clicked();

    void on_pushButton_TRangeRead_clicked();

signals:
    void HistoryChange(qint8 index);

private:
    Ui::Widget *ui;
    QSerialPort *my_serialport;
    quint8 ID;
    char SuperID[4];
    quint16 TXcnt;
    quint16 RXcnt;
    quint8 func;
    quint8 ADD;
    quint8 Data_H;
    quint8 Data_L;
    QTimer *timer;
    QByteArray m_readData;
    QLineEdit *LineEdit_real[30];
    QLineEdit *LineEdit_line[30];
    QLineEdit *LineEdit_lineNum;
    QTextEdit *TextEdit_History;
    QButtonGroup* btnGroupPeak;
    QButtonGroup* btnGroupAlarmLow;
    QButtonGroup* btnGroupAlarmHigh;

};

#endif // WIDGET_H
