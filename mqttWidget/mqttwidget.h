#ifndef MQTTWIDGET_H
#define MQTTWIDGET_H

#include <QQueue>
#include <QWidget>
#include <QtMqtt/QMqttClient>

#include "websocketiodevice.h"

namespace Ui {
class MqttWidget;
}

class MqttWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MqttWidget(QWidget *parent = 0);
    ~MqttWidget();

    void createMQTTClient();
    void setClientPort(int p);

public slots:
    void publishMQTT();
    void connectMQTT();
    void slotSelectFileForSend();
    void slotRun();
protected:
    void loadIniFile();
    void saveIniFile();
protected slots:
    void updateLogStateChange();
    void brokerDisconnected();

private:
    Ui::MqttWidget *ui;
    QString currentFilePath;
    QMqttClient *m_client;

    QQueue<QByteArray> queue;
};

#endif // MQTTWIDGET_H
