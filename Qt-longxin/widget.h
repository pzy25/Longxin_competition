#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QChartView>
#include <QValueAxis>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QModbusTcpClient>
#include <QModbusReply>
#include <QMessageBox>
#include <QVector>
#include <QtMqtt/qmqttclient.h>
#include <QMessageAuthenticationCode>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include "dialog1.h"

using namespace QT_CHARTS_NAMESPACE;
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void InitiatChart();
    void InitiatModClient();
    void InitiatMQTT();
    void InitTime();
    void uploadali();
    void uploadali2();
    void uploadali3();
    void uploadali4();
    void generaChart();
    void alarm();
    void InitLogin();
    void ReadDeviceData(QModbusTcpClient *client,int deviceNumber);
    void processData(const QModbusDataUnit &readData, int deviceNumber);
    void ReadyRead();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_timer0_clicked();

    void handtimeout();

//    void ReadyRead();
    void sendRequest(QModbusTcpClient *client, int deviceNumber);

    void connectMqttSuccess();

    void receiveMessageSlot(const QByteArray &ba,const QMqttTopicName &topic);

    void on_pushButton_5_clicked();

    void on_swich1Button_clicked();

    void receiveAssure1(bool flagsuccess);


    void on_swich2Button_clicked();

    void on_swich3Button_clicked();

    void on_swich4Button_clicked();

private:
    Ui::Widget *ui;
    QTimer *timer;

    //chart function
    QLineSeries* m_lineSeries1,* m_lineSeries2,* m_lineSeries3,* m_lineSeries4;
    QValueAxis *m_axisX1, *m_axisY1;
    QValueAxis *m_axisX2, *m_axisY2;
    QValueAxis *m_axisX3, *m_axisY3;
    QValueAxis *m_axisX4, *m_axisY4;
    QChart* m_chart1,*m_chart2,*m_chart3,*m_chart4;

    const int AXIS_MAX_X1=10,AXIS_MAX_Y1=50;
    const int AXIS_MAX_X2=10,AXIS_MAX_Y2=100;
    const int AXIS_MAX_X3=10,AXIS_MAX_Y3=250;
    const int AXIS_MAX_X4=10;
    const double AXIS_MAX_Y4=0.5;

    int pointCount = 0;

    //modbus tcp client
    QList<QModbusTcpClient*> clients;
    QVector<quint16> vector;
    quint16 temperature;
    quint16 humidty;
    qreal voltage;
    qreal current;
    qreal voltali;
    qreal currali;
    //flag
    int flagswich = 0,flagswich2 = 0,flagswich3 = 0,flagswich4 = 0;
    int flagtemp = 0;

    //MQTT
       QMqttClient *m_client;
       QString m_strProductKey;
       QString m_strDeviceName;
       QString m_strDeviceSecret;
       QString m_strRegionId;
       QString m_strTargetServer;
    //Login
       Dialog1 *my_login;
       int flagLogin = 0;

};
#endif // WIDGET_H
