#-------------------------------------------------
#
# Project created by QtCreator 2023-04-22T07:48:13
#
#-------------------------------------------------

QT       += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mqttWidget
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mqttwidget.cpp \
        console.cpp

HEADERS += \
        mqttwidget.h \
        console.h

FORMS += \
        mqttwidget.ui

MOC_DIR = moc
UI_DIR = ui
OBJECTS_DIR = obj

unix {
    target.path = "/usr/local/mqtt/bin"
    INSTALLS += target
}


INCLUDEPATH += ../qtmqtt/include
    
LIBS += -L"../qtmqtt/lib"
LIBS += -lQt5Mqtt

RESOURCES += \
    resors.qrc

DISTFILES += \
    /images/SendEmail.ico \
    /images/mqttWidget.rc
