#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    timer->setInterval(1000);
    InitTime();
    InitiatChart();
    InitiatModClient();
    InitiatMQTT();
    InitLogin();
    connect(timer,&QTimer::timeout,this,&Widget::handtimeout);
    connect(timer,&QTimer::timeout,this,&Widget::InitTime);
    connect(timer, &QTimer::timeout, [&]() {
          static int currentDevice = 0;  // 当前设备索引
          // 发送当前设备请求
          sendRequest(clients[currentDevice], currentDevice + 1);
          // 移动到下一个设备
          currentDevice++;
          if (currentDevice >= clients.length()) {
              currentDevice = 0;  // 返回第一个设备
          }
      });

    for (QModbusTcpClient* client : clients) {
                connect(client, &QModbusTcpClient::stateChanged, [=](QModbusReply *reply) {
                if (reply->error() == QModbusDevice::NoError) {
                    const QModbusDataUnit data = reply->result();
                    processData(data, clients.indexOf(client) + 1);  // 处理设备数据
                } else {
                    qDebug() << "Read request error for device:" << clients.indexOf(client) + 1 << reply->errorString();
                }
                reply->deleteLater();
            });
        }

}
Widget::~Widget()
{
    m_client->disconnectFromHost();
    delete ui;
}

void Widget::InitiatChart()
{
    m_axisX1 = new QValueAxis();
    m_axisY1 = new QValueAxis();
   // m_axisX1->setTitleText("X-时间");
    m_axisY1->setTitleText("Y-温度");
    m_axisX1->setMin(0);
    m_axisY1->setMin(0);
    m_axisX1->setMax(AXIS_MAX_X1);
    m_axisY1->setMax(AXIS_MAX_Y1);

    m_axisX2 = new QValueAxis();
    m_axisY2 = new QValueAxis();
   // m_axisX2->setTitleText("X-时间");
    m_axisY2->setTitleText("Y-湿度");
    m_axisX2->setMin(0);
    m_axisY2->setMin(0);
    m_axisX2->setMax(AXIS_MAX_X2);
    m_axisY2->setMax(AXIS_MAX_Y2);

    m_axisX3 = new QValueAxis();
    m_axisY3 = new QValueAxis();
   // m_axisX3->setTitleText("X-时间");
    m_axisY3->setTitleText("Y-电压");
    m_axisX3->setMin(0);
    m_axisY3->setMin(0);
    m_axisX3->setMax(AXIS_MAX_X3);
    m_axisY3->setMax(AXIS_MAX_Y3);

    m_axisX4 = new QValueAxis();
    m_axisY4 = new QValueAxis();
   // m_axisX4->setTitleText("X-时间");
    m_axisY4->setTitleText("Y-电流");
    m_axisX4->setMin(0);
    m_axisY4->setMin(0);
    m_axisX4->setMax(AXIS_MAX_X4);
    m_axisY4->setMax(AXIS_MAX_Y4);

    m_lineSeries1 = new QLineSeries();                             // 创建曲线绘制对象
    m_lineSeries1->setPointsVisible(true);                         // 设置数据点可见
    m_lineSeries1->setName("充电桩1");                            // 图例名称

    m_lineSeries2 = new QLineSeries();                             // 创建曲线绘制对象
    m_lineSeries2->setPointsVisible(true);                         // 设置数据点可见
    m_lineSeries2->setName("充电桩1");                            // 图例名称

    m_lineSeries3 = new QLineSeries();                             // 创建曲线绘制对象
    m_lineSeries3->setPointsVisible(true);                         // 设置数据点可见
    m_lineSeries3->setName("充电桩1");                            // 图例名称

    m_lineSeries4 = new QLineSeries();                             // 创建曲线绘制对象
    m_lineSeries4->setPointsVisible(true);                         // 设置数据点可见
    m_lineSeries4->setName("充电桩1");                            // 图例名称

    m_chart1 = new QChart();                                        // 创建图表对象
    m_chart1->addAxis(m_axisX1, Qt::AlignBottom);                      // 将X轴添加到图表上
    m_chart1->addAxis(m_axisY1, Qt::AlignLeft);                    // 将Y轴添加到图表上
    m_chart1->addSeries(m_lineSeries1);                              // 将曲线对象添加到图表上

    m_chart2 = new QChart();                                        // 创建图表对象
    m_chart2->addAxis(m_axisX2, Qt::AlignBottom);                      // 将X轴添加到图表上
    m_chart2->addAxis(m_axisY2, Qt::AlignLeft);                    // 将Y轴添加到图表上
    m_chart2->addSeries(m_lineSeries2);                              // 将曲线对象添加到图表上

    m_chart3 = new QChart();                                        // 创建图表对象
    m_chart3->addAxis(m_axisX3, Qt::AlignBottom);                      // 将X轴添加到图表上
    m_chart3->addAxis(m_axisY3, Qt::AlignLeft);                    // 将Y轴添加到图表上
    m_chart3->addSeries(m_lineSeries3);                              // 将曲线对象添加到图表上

    m_chart4 = new QChart();                                        // 创建图表对象
    m_chart4->addAxis(m_axisX4, Qt::AlignBottom);                      // 将X轴添加到图表上
    m_chart4->addAxis(m_axisY4, Qt::AlignLeft);                    // 将Y轴添加到图表上
    m_chart4->addSeries(m_lineSeries4);                              // 将曲线对象添加到图表上

    m_chart1->setAnimationOptions(QChart::SeriesAnimations);        // 动画：能使曲线绘制显示的更平滑，过渡效果更好看
    m_chart2->setAnimationOptions(QChart::SeriesAnimations);        // 动画：能使曲线绘制显示的更平滑，过渡效果更好看
    m_chart3->setAnimationOptions(QChart::SeriesAnimations);        // 动画：能使曲线绘制显示的更平滑，过渡效果更好看
    m_chart4->setAnimationOptions(QChart::SeriesAnimations);        // 动画：能使曲线绘制显示的更平滑，过渡效果更好看

    m_lineSeries1->attachAxis(m_axisX1);                             // 曲线对象关联上X轴，此步骤必须在m_chart->addSeries之后
    m_lineSeries1->attachAxis(m_axisY1);                             // 曲线对象关联上Y轴，此步骤必须在m_chart->addSeries之后
    m_lineSeries2->attachAxis(m_axisX2);                             // 曲线对象关联上X轴，此步骤必须在m_chart->addSeries之后
    m_lineSeries2->attachAxis(m_axisY2);                             // 曲线对象关联上Y轴，此步骤必须在m_chart->addSeries之后
    m_lineSeries3->attachAxis(m_axisX3);                             // 曲线对象关联上X轴，此步骤必须在m_chart->addSeries之后
    m_lineSeries3->attachAxis(m_axisY3);                             // 曲线对象关联上Y轴，此步骤必须在m_chart->addSeries之后
    m_lineSeries4->attachAxis(m_axisX4);                             // 曲线对象关联上X轴，此步骤必须在m_chart->addSeries之后
    m_lineSeries4->attachAxis(m_axisY4);                             // 曲线对象关联上Y轴，此步骤必须在m_chart->addSeries之后

    ui->chartView1->setChart(m_chart1);
    ui->chartView1->setRenderHint(QPainter::Antialiasing);

    ui->chartView2->setChart(m_chart2);
    ui->chartView2->setRenderHint(QPainter::Antialiasing);

    ui->chartView3->setChart(m_chart3);
    ui->chartView3->setRenderHint(QPainter::Antialiasing);

    ui->chartView4->setChart(m_chart4);
    ui->chartView4->setRenderHint(QPainter::Antialiasing);


}

void Widget::InitiatModClient()
{
        QList<QModbusTcpClient*> clients;
        clients.append(new QModbusTcpClient());
        clients.append(new QModbusTcpClient());
        clients.append(new QModbusTcpClient());
        clients.append(new QModbusTcpClient());
        // 设置每个客户端的Modbus TCP服务器地址和端口
        const QString ipAddress1 = "192.168.0.1";
        const int port1 = 501;
        clients[0]->setConnectionParameter(QModbusDevice::NetworkAddressParameter, ipAddress1);
        clients[0]->setConnectionParameter(QModbusDevice::NetworkPortParameter, port1);

        const QString ipAddress2 = "192.168.0.2";
        const int port2 = 502;
        clients[1]->setConnectionParameter(QModbusDevice::NetworkAddressParameter, ipAddress2);
        clients[1]->setConnectionParameter(QModbusDevice::NetworkPortParameter, port2);

        const QString ipAddress3 = "192.168.0.3";
        const int port3 = 503;
        clients[2]->setConnectionParameter(QModbusDevice::NetworkAddressParameter, ipAddress3);
        clients[2]->setConnectionParameter(QModbusDevice::NetworkPortParameter, port3);

        const QString ipAddress4 = "192.168.0.3";
        const int port4 = 504;
        clients[2]->setConnectionParameter(QModbusDevice::NetworkAddressParameter, ipAddress4);
        clients[2]->setConnectionParameter(QModbusDevice::NetworkPortParameter, port4);



        // 连接到Modbus TCP服务器
        for (QModbusTcpClient* client : clients) {
            if (!client->connectDevice()) {
                qDebug() << "Failed to connect to Modbus TCP server!";
            }
        }

}

void Widget::InitiatMQTT()
{
    m_client = new QMqttClient(this);
    m_client->setAutoKeepAlive(true);

    m_strProductKey="ie44FqhJNgx";  //需要跟阿里云Iot平台一致;
    m_strDeviceName="D001";   //需要跟阿里云Iot平台一致;
    m_strDeviceSecret="867e361641c03ffdf283fd4bccc593c8";   //需要跟阿里云平台一致
    m_strRegionId="cn-shanghai";
    m_strTargetServer = m_strProductKey + ".iot-as-mqtt." + m_strRegionId + ".aliyuncs.com";//域名
    QString clientId="abcdefg";       //表示客户端ID，建议使用设备的MAC地址或SN码，64字符内。
    QString signmethod = "hmacsha1";    //加密方式
    QString message ="clientId"+clientId+"deviceName"+m_strDeviceName+"productKey"+m_strProductKey;
    m_client->setUsername(m_strDeviceName + "&" + m_strProductKey);
    m_client->setClientId(clientId + "|securemode=3,signmethod=" + signmethod + /*",timestamp="+timestamp+ */"|");
    m_client->setPassword(QMessageAuthenticationCode::hash(message.toLocal8Bit(),m_strDeviceSecret.toLocal8Bit(),
                                                           QCryptographicHash::Sha1).toHex());

    m_client->setHostname(m_strTargetServer);
    m_client->setPort(1883);

    QString subscription="/ie44FqhJNgx/D001/user/get";
    QString topic="/ie44FqhJNgx/D001/user/update";
    m_client->subscribe(subscription);

    m_client->connectToHost();
    qDebug()<<"lianjie"<<endl;

    connect(m_client,&QMqttClient::connected,this,&Widget::connectMqttSuccess);
}

void Widget::on_pushButton_clicked()//回到首页
{
    ui->stackedWidget->setCurrentIndex(0);
}

void Widget::on_pushButton_2_clicked()//实时监控
{
    ui->stackedWidget->setCurrentIndex(1);
}

void Widget::on_pushButton_3_clicked()//图表
{
    ui->stackedWidget->setCurrentIndex(2);
}

void Widget::handtimeout()
{
    ReadyRead();
}
void Widget::sendRequest(QModbusTcpClient *client, int deviceNumber)
{
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0, 10);
       if (auto *reply = client->sendReadRequest(readUnit, deviceNumber)) {
           // 请求发送成功
           if (!reply->isFinished()) {
               // 等待读取完成
               QObject::connect(reply, &QModbusReply::finished, reply, &QObject::deleteLater);
           }
       } else {
           qDebug() << "Failed to send read request for device" << deviceNumber;
       }
}

void Widget::ReadyRead()
{
    generaChart();
    uploadali();
    uploadali2();
    uploadali3();
    uploadali4();
    alarm();
}
void Widget::connectMqttSuccess()
{
    qDebug()<<"连接阿里云成功"<<endl;

    connect(m_client,&QMqttClient::messageReceived,this,&Widget::receiveMessageSlot);
    connect(m_client,&QMqttClient::disconnected,[this]()
    {
        QMessageBox::warning(this,"连接提示","服务器断开");
    });
}

void Widget::receiveMessageSlot(const QByteArray &ba,const QMqttTopicName &topic)
{
       QString str = topic.name()+QString(ba);
       QString json_str=QString(ba);

       QJsonParseError jsonError;
       QJsonDocument doc = QJsonDocument::fromJson(json_str.toUtf8(), &jsonError);

       if (doc.isObject()) {
               QJsonObject object = doc.object();  // 转化为对象

               QJsonValue value = object.value("data");
               int data= value.toInt();
               QString num=QString::number(data);

               QJsonValue name = object.value("cmd");  // 获取指定 key 对应的 value
               QString cmd = name.toString();// 将 value 转化为字符串

               if(cmd=="NO1"){
                  if( (data&&(!flagswich)) | ((!data)&&flagswich) )
                    on_swich1Button_clicked();
               }
               else if(cmd=="NO2"){
                   if( (data&&(!flagswich2)) | ((!data)&&flagswich2) )
                    on_swich2Button_clicked();
               }
               else if(cmd=="NO3"){
                  if( (data&&(!flagswich3)) | ((!data)&&flagswich3) )
                    on_swich3Button_clicked();
               }
               else if(cmd=="NO4"){
                   if( (data&&(!flagswich4)) | ((!data)&&flagswich4) )
                    on_swich4Button_clicked();
               }


        }

}


void Widget::generaChart()
{
    if(pointCount > AXIS_MAX_X1)
    {
        m_lineSeries1->remove(0);
        m_lineSeries2->remove(0);
        m_lineSeries3->remove(0);
        m_lineSeries4->remove(0);
        m_chart1->axes(Qt::Horizontal).back()->setMin(pointCount - AXIS_MAX_X1);
        m_chart1->axes(Qt::Horizontal).back()->setMax(pointCount);                    // 更新X轴范围
        m_chart2->axes(Qt::Horizontal).back()->setMin(pointCount - AXIS_MAX_X2);
        m_chart2->axes(Qt::Horizontal).back()->setMax(pointCount);                    // 更新X轴范围
        m_chart3->axes(Qt::Horizontal).back()->setMin(pointCount - AXIS_MAX_X3);
        m_chart3->axes(Qt::Horizontal).back()->setMax(pointCount);                    // 更新X轴范围
        m_chart4->axes(Qt::Horizontal).back()->setMin(pointCount - AXIS_MAX_X4);
        m_chart4->axes(Qt::Horizontal).back()->setMax(pointCount);                    // 更新X轴范围
    }
    m_lineSeries1->append(QPointF(pointCount, ui->temp1->text().mid(0,2).toInt()));
    m_lineSeries2->append(QPointF(pointCount, ui->hum1->text().mid(0,2).toInt()));
    m_lineSeries3->append(QPointF(pointCount, ui->vol1->text().toDouble()));//220
    m_lineSeries4->append(QPointF(pointCount, ui->current1->text().toDouble()));//0.13

    pointCount++;
}

void Widget::alarm()
{
    if(!flagtemp)
      if((ui->temp1->text().mid(0,2).toInt())>28)
      {
          QMessageBox::warning(this,"报警","充电桩1的温度过高");
          flagtemp = 1;
          qDebug()<<ui->temp1->text()<<endl;
      }

}

void Widget::InitLogin()
{
    my_login = new Dialog1;
    connect(my_login,&Dialog1::assure1,this,&Widget::receiveAssure1);
    ui->pushButton_4->setText("浏览模式");
    ui->swich1Button->setEnabled(false);
    ui->swich2Button->setEnabled(false);
    ui->swich3Button->setEnabled(false);
    ui->swich4Button->setEnabled(false);
    // ui->swich2->setEnabled(false);
}

void Widget::ReadDeviceData(QModbusTcpClient *client, int deviceNumber)
{
    // 发送读取请求
       QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0, 10);
       if (auto *reply = client->sendReadRequest(readUnit, deviceNumber)) {
           // 请求发送成功
           if (!reply->isFinished()) {
               // 等待读取完成
               QObject::connect(reply, &QModbusReply::finished, reply, &QObject::deleteLater);
           }
       } else {
           qDebug() << "Failed to send read request for device" << deviceNumber;
       }
}

void Widget::processData(const QModbusDataUnit &readData, int deviceNumber)
{
    temperature =readData.value(0);
       humidty = readData.value(1);
       voltage = (readData.value(2)*16*1024*1024+readData.value(3)*64*1024+readData.value(4)*256+\
                           readData.value(5))/10000.0;
       current = (readData.value(6)*16*1024*1024+readData.value(7)*64*1024+readData.value(8)*256+\
                           readData.value(9))/10000.0;


                   QString str1 =QString("%1%2").arg(temperature).arg("°C");
                   QString str2 =QString("%1%2").arg(humidty).arg("%");
                   QString str3 =QString("%1%2").arg(voltage,0,'f',2).arg("V");
                   QString str4 =QString("%1%2").arg(current,0,'f',2).arg("A");

        switch(deviceNumber)
        {
            case 1:
                   ui->temp1->setText(str1);
                   ui->hum1->setText(str2);
                   ui->vol1->setText(str3);
                   ui->current1->setText(str4);
                   break;
            case 2:
                   ui->temp2->setText(str1);
                   ui->hum2->setText(str2);
                   ui->vol2->setText(str3);
                   ui->current2->setText(str4);
                   break;
            case 3:
                   ui->temp3->setText(str1);
                   ui->hum3->setText(str2);
                   ui->vol3->setText(str3);
                   ui->current3->setText(str4);
                   break;
            case 4:
                   ui->temp4->setText(str1);
                   ui->hum4->setText(str2);
                   ui->vol4->setText(str3);
                   ui->current4->setText(str4);
                   break;
        }
}

void Widget::uploadali()
{
    int aliswich;
    if(ui->swich1->text()=="OFF")
    {
        aliswich = 0;
    }
    else
    {
        aliswich = 1;
    }
    QString topic="/ie44FqhJNgx/D001/user/update";
    QString msg="{\"temperature1\":";
            msg+=ui->temp1->text().mid(0,2);
            msg+=",\"Humidity1\":";
             msg+=ui->hum1->text().mid(0,2);
              msg+=",\"RMSCurrent1\":";
              msg+=ui->current1->text();
              msg+=",\"RMSVoltage1\":";
              msg+=ui->vol1->text();
              msg+=",\"NO1\":";
              msg+=QString::number(aliswich);
              msg+=",\"GeoLocation1\":";
              msg+=QString::number(aliswich);
              msg+="}";
    QByteArray ba;
    ba.append(msg);
    m_client->publish(topic,ba);
}

void Widget::uploadali2()
{
    int aliswich;
    if(ui->swich2->text()=="OFF")
    {
        aliswich = 0;
    }
    else
    {
        aliswich = 1;
    }
    QString topic="/ie44FqhJNgx/D001/user/update";
    QString msg="{\"temperature2\":";
            msg+=ui->temp2->text().mid(0,2);
            msg+=",\"Humidity2\":";
             msg+=ui->hum2->text().mid(0,2);
              msg+=",\"RMSCurrent2\":";
              msg+=ui->current2->text();
              msg+=",\"RMSVoltage2\":";
              msg+=ui->vol2->text();
              msg+=",\"NO2\":";
              msg+=QString::number(aliswich);
              msg+=",\"GeoLocation2\":";
              msg+=QString::number(aliswich);
              msg+="}";
    QByteArray ba;
    ba.append(msg);
    m_client->publish(topic,ba);
}

void Widget::uploadali3()
{
    int aliswich;
    if(ui->swich3->text()=="OFF")
    {
        aliswich = 0;
    }
    else
    {
        aliswich = 1;
    }
    QString topic="/ie44FqhJNgx/D001/user/update";
    QString msg="{\"temperature3\":";
            msg+=ui->temp3->text().mid(0,2);
            msg+=",\"Humidity3\":";
             msg+=ui->hum3->text().mid(0,2);
              msg+=",\"RMSCurrent3\":";
              msg+=ui->current3->text();
              msg+=",\"RMSVoltage3\":";
              msg+=ui->vol3->text();
              msg+=",\"NO3\":";
              msg+=QString::number(aliswich);
              msg+=",\"GeoLocation3\":";
              msg+=QString::number(aliswich);
              msg+="}";
    QByteArray ba;
    ba.append(msg);
    m_client->publish(topic,ba);
}

void Widget::uploadali4()
{
    int aliswich;
    if(ui->swich4->text()=="OFF")
    {
        aliswich = 0;
    }
    else
    {
        aliswich = 1;
    }
    QString topic="/ie44FqhJNgx/D001/user/update";
    QString msg="{\"temperature4\":";
            msg+=ui->temp4->text().mid(0,2);
            msg+=",\"Humidity4\":";
             msg+=ui->hum4->text().mid(0,2);
              msg+=",\"RMSCurrent4\":";
              msg+=ui->current4->text();
              msg+=",\"RMSVoltage4\":";
              msg+=ui->vol4->text();
              msg+=",\"NO4\":";
              msg+=QString::number(aliswich);
              msg+=",\"GeoLocation4\":";
              msg+=QString::number(aliswich);
              msg+="}";
    QByteArray ba;
    ba.append(msg);
    m_client->publish(topic,ba);
}

void Widget::InitTime()
{

       QDateTime time = QDateTime::currentDateTime();
       ui->time_hour->setText(time.toString("hh"));
       ui->time_min->setText(time.toString("mm"));
       ui->date->setText(time.toString("M月dd日"));
       ui->week->setText(time.toString("dddd"));

}

void Widget::on_pushButton_4_clicked()//另一个界面
{
    if(!flagLogin)
    {
        my_login->show();
    }
    else
    {
        flagLogin = 0;
        ui->swich1Button->setEnabled(false);
        ui->swich2Button->setEnabled(false);
        ui->swich3Button->setEnabled(false);
        ui->swich4Button->setEnabled(false);
       // ui->swich2->setEnabled(false);
        ui->pushButton_4->setText("浏览模式");
    }
}

void Widget::on_timer0_clicked()  // start timer or stop timer
{
    if(timer->isActive())
    {
        timer->stop();
        ui->timer0->setText("启动定时器");
    }else
    {
        pointCount = 0;
        timer->start(1000);
        ui->timer0->setText("停止定时器");
    }
}

void Widget::on_pushButton_5_clicked()//clear chart
{
    m_lineSeries1->clear();
    m_lineSeries2->clear();
    m_lineSeries3->clear();
    m_lineSeries4->clear();

    m_chart1->axes(Qt::Horizontal).back()->setMin(0);
    m_chart1->axes(Qt::Horizontal).back()->setMax(AXIS_MAX_X1);
    m_chart2->axes(Qt::Horizontal).back()->setMin(0);
    m_chart2->axes(Qt::Horizontal).back()->setMax(AXIS_MAX_X2);
    m_chart3->axes(Qt::Horizontal).back()->setMin(0);
    m_chart3->axes(Qt::Horizontal).back()->setMax(AXIS_MAX_X3);
    m_chart4->axes(Qt::Horizontal).back()->setMin(0);
    m_chart4->axes(Qt::Horizontal).back()->setMax(AXIS_MAX_X3);

    pointCount = 0;
}

void Widget::on_swich1Button_clicked()//充放电
{
    if(!flagswich)
    {
        QModbusDataUnit writeUnit(QModbusDataUnit::Coils,0,1);
        writeUnit.setValue(0,1);
        QModbusReply *reply = clients[0]->sendWriteRequest(writeUnit,1);
        if(reply){

            reply->deleteLater();
        }
        flagswich = 1;
        ui->swich1->setText("ON");
        ui->swich1Button->setText("断电");
    }
    else
    {
        QModbusDataUnit writeUnit(QModbusDataUnit::Coils,0,1);
        writeUnit.setValue(0,0);
        QModbusReply *reply = clients[0]->sendWriteRequest(writeUnit,1);
        if(reply){

            reply->deleteLater();
        }
         flagswich = 0;
         ui->swich1->setText("OFF");
        ui->swich1Button->setText("充电");
        //断电后不显示电压电流
        ui->vol1->setText("0.00");
        ui->current1->setText("0.00");
    }
}

void Widget::receiveAssure1(bool flagsuccess)
{
    if(flagsuccess)
    {
       qDebug()<<"登陆成功"<<endl;
       my_login->close();
       flagLogin = 1;
       ui->swich1Button->setEnabled(flagLogin);
       ui->swich2Button->setEnabled(flagLogin);
       ui->swich3Button->setEnabled(flagLogin);
       ui->swich4Button->setEnabled(flagLogin);
       ui->pushButton_4->setText("管理员模式");
    }
    else
    {
       qDebug()<<"登陆失败"<<endl;

    }
}

void Widget::on_swich2Button_clicked()
{
    if(!flagswich2)
    {
        QModbusDataUnit writeUnit(QModbusDataUnit::Coils,0,1);
        writeUnit.setValue(0,1);
        QModbusReply *reply = clients[2]->sendWriteRequest(writeUnit,1);
        if(reply){

            reply->deleteLater();
        }
        flagswich2 = 1;
        ui->swich2->setText("ON");
        ui->swich2Button->setText("断电");
    }
    else
    {
        QModbusDataUnit writeUnit(QModbusDataUnit::Coils,0,1);
        writeUnit.setValue(0,0);
        QModbusReply *reply = clients[2]->sendWriteRequest(writeUnit,1);
        if(reply){

            reply->deleteLater();
        }
         flagswich2 = 0;
         ui->swich2->setText("OFF");
        ui->swich2Button->setText("充电");
        //断电后不显示电压电流
        ui->vol2->setText("0.00");
        ui->current2->setText("0.00");
    }
}

void Widget::on_swich3Button_clicked()
{
    if(!flagswich3)
    {
        QModbusDataUnit writeUnit(QModbusDataUnit::Coils,0,1);
        writeUnit.setValue(0,1);
        QModbusReply *reply = clients[3]->sendWriteRequest(writeUnit,1);
        if(reply){

            reply->deleteLater();
        }
        flagswich3 = 1;
        ui->swich3->setText("ON");
        ui->swich3Button->setText("断电");
    }
    else
    {
        QModbusDataUnit writeUnit(QModbusDataUnit::Coils,0,1);
        writeUnit.setValue(0,0);
        QModbusReply *reply = clients[3]->sendWriteRequest(writeUnit,1);
        if(reply){

            reply->deleteLater();
        }
        flagswich3 = 0;
        ui->swich3->setText("OFF");
        ui->swich3Button->setText("充电");
        //断电后不显示电压电流
        ui->vol3->setText("0.00");
        ui->current3->setText("0.00");
    }

}

void Widget::on_swich4Button_clicked()
{
    if(!flagswich4)
    {
        QModbusDataUnit writeUnit(QModbusDataUnit::Coils,0,1);
        writeUnit.setValue(0,1);
        QModbusReply *reply = clients[4]->sendWriteRequest(writeUnit,1);
        if(reply){

            reply->deleteLater();
        }
        flagswich4 = 1;
        ui->swich4->setText("ON");
        ui->swich4Button->setText("断电");
    }
    else
    {
        QModbusDataUnit writeUnit(QModbusDataUnit::Coils,0,1);
        writeUnit.setValue(0,0);
        QModbusReply *reply = clients[4]->sendWriteRequest(writeUnit,1);
        if(reply){

            reply->deleteLater();
        }
        flagswich4 = 0;
        ui->swich4->setText("OFF");
        ui->swich4Button->setText("充电");
        //断电后不显示电压电流
        ui->vol4->setText("0.00");
        ui->current4->setText("0.00");
    }
}
