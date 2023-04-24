#include "mqttwidget.h"
#include "ui_mqttwidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <qdir.h>
#include <qsettings.h>

#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(lcWebSocketMqtt, "qtdemo.websocket.mqtt")

MqttWidget::MqttWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MqttWidget)
{
    ui->setupUi(this);
    loadIniFile();
    m_client = nullptr;
    connect(ui->selectFilePathTB, SIGNAL(clicked()), this, SLOT(slotSelectFileForSend()));
    connect(ui->runTB, SIGNAL(clicked()), this, SLOT(slotRun()));

    createMQTTClient();
}

MqttWidget::~MqttWidget()
{
    saveIniFile();
    delete ui;
}

void MqttWidget::createMQTTClient()
{
    m_client = new QMqttClient(this);
    m_client->setHostname(ui->hostLE->text());
    m_client->setPort(ui->portSB->value());

    connect(m_client, &QMqttClient::stateChanged, this, &MqttWidget::updateLogStateChange);
    connect(m_client, &QMqttClient::disconnected, this, &MqttWidget::brokerDisconnected);

    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        const QString content = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")
                + QLatin1String(" Received Topic: ")
                + topic.name()
                + QLatin1String(" Message: ")
                + message;
        ui->console->putData(content.toLocal8Bit());
    });

    connect(m_client, &QMqttClient::pingResponseReceived, this, [this]() {
        const QString content = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")
                + QLatin1String(" PingResponse");
        ui->console->putData(content.toLocal8Bit());
    });

    connect(m_client, &QMqttClient::errorChanged, this, [this](const QMqttClient::ClientError e) {
        if (e == QMqttClient::NoError)
            return;
        const QString content = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")
                + QLatin1String(" Error Occurred: ")
                + e
                + QLatin1String(" Client state: ")
                + m_client->state();
        m_client->disconnectFromHost();
    });

    connect(ui->hostLE, &QLineEdit::textChanged,
            m_client, &QMqttClient::setHostname);
    connect(ui->portSB, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MqttWidget::setClientPort);
}

void MqttWidget::setClientPort(int p)
{
    m_client->setPort(p);
}

void MqttWidget::publishMQTT()
{
    QString filePath = ui->filePathLE->text();
    QFile file(filePath);
    if(!file.exists())
    {
        QMessageBox::critical(this, tr("Sending file"), tr("there is no access to the file %0 for sending").arg(filePath));
        return;
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, tr("Sending file"), tr("the file %0 cannot be open for reading").arg(filePath));
        return;
    }

    if (file.size() == 0)
    {
        QMessageBox::critical(this, tr("Sending file"), tr("the file %0 for sending is empty").arg(filePath));
        return;
    }

    if(m_client->state() == QMqttClient::Connected)
    {
        int count_out = 0;
        while (!file.atEnd()) {
            const QByteArray line = file.readLine();
            if(line.size() != 0)
            {
                ++count_out;
                m_client->publish(ui->topicLE->text(), line, 0, true);
            }
        }
        ui->console->putData(QString("Number of lines sent %0").arg(count_out).toLocal8Bit());
    }
    else
    {
        while (!file.atEnd()) {
            const QByteArray line = file.readLine();
            queue.enqueue(line);
        }
        ui->console->putData(QString("File transfer delayed until joining broker").toLocal8Bit());
    }
}

void MqttWidget::connectMQTT()
{
    m_client->setHostname(ui->hostLE->text());
    m_client->setPort(ui->portSB->value());

    if (m_client->state() == QMqttClient::Disconnected) {
        ui->hostLE->setEnabled(false);
        ui->portSB->setEnabled(false);
        m_client->connectToHost();
    } else {
        ui->hostLE->setEnabled(true);
        ui->portSB->setEnabled(true);
        m_client->disconnectFromHost();
    }
}

void MqttWidget::slotSelectFileForSend()
{
    if(currentFilePath.isEmpty())
        currentFilePath = QDir::homePath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File For Send"), currentFilePath, tr("*.*"));
    if(!fileName.isEmpty())
    {
        ui->filePathLE->setText(fileName);
        currentFilePath = QFileInfo(fileName).path();
    }
}

void MqttWidget::slotRun()
{
    QString filePath = ui->filePathLE->text();
    if(filePath.isEmpty())
    {
        QMessageBox::critical(this, tr("Sending file"), tr("the file %0 for sending is not chosen").arg(filePath));
        return;
    }
    connectMQTT();
    publishMQTT();
}

void MqttWidget::loadIniFile()
{
    QSettings settings(QDir::home().absoluteFilePath(".config/mqtt/mqtt.ini"), QSettings::IniFormat);
    setGeometry( settings.value("MainWin/geometry", QRect(100,100,100,100)).toRect());

    ui->hostLE->setText(settings.value("MainWin/host","test.mosquitto.org").toString());
    ui->portSB->setValue(settings.value("MainWin/port",1883).toInt());
    ui->topicLE->setText(settings.value("MainWin/topic","piklema/test").toString());
    ui->usernameLE->setText(settings.value("MainWin/username","").toString());

    currentFilePath = settings.value("MainWin/currentFilePath",QDir::homePath()).toString();
    ui->splitter->restoreState(settings.value("MainWin/splitter").toByteArray());
}

void MqttWidget::saveIniFile()
{
    QSettings settings(QDir::home().absoluteFilePath(".config/mqtt/mqtt.ini"), QSettings::IniFormat);
    settings.setValue("MainWin/geometry", geometry() );

    settings.setValue("MainWin/host", ui->hostLE->text() );
    settings.setValue("MainWin/port", ui->portSB->value() );
    settings.setValue("MainWin/topic", ui->topicLE->text() );
    settings.setValue("MainWin/username", ui->usernameLE->text() );

    settings.setValue("MainWin/currentFilePath", currentFilePath );
    settings.setValue("MainWin/splitter", ui->splitter->saveState());
}

void MqttWidget::updateLogStateChange()
{
    QString state;
    switch (m_client->state()) {
    case 0:
    {
        state = "Disconnected";
        break;
    }
    case 1:
    {
        state = "Connecting";
        break;
    }
    case 2:
    {
        state = "Connected";
        break;
    }
    }
    const QString content = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")
            + QLatin1String(": State Change - ")
            + state;
    ui->console->putData(content.toLocal8Bit());

    if(m_client->state() == QMqttClient::Connected)
    {
        if(queue.size() != 0)
        {
            int count_out = 0;
            while (!queue.isEmpty()) {
                const QByteArray line = queue.dequeue();
                if(line.size() != 0)
                {
                    ++count_out;
                    m_client->publish(ui->topicLE->text(), line, 0,true);
                }
            }
            ui->console->putData(QString("Number of lines sent %0").arg(count_out).toLocal8Bit());
        }
        connectMQTT();
    }
}

void MqttWidget::brokerDisconnected()
{
    ui->hostLE->setEnabled(true);
    ui->portSB->setEnabled(true);
}

