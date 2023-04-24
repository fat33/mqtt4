#include "mqttwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MqttWidget w;
    w.show();

    return a.exec();
}
