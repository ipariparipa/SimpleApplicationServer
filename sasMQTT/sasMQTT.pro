include(../global.pri)

TEMPLATE = lib
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasJSON/include

CONFIG(SAS_LOG4CXX_ENABLED) {
    LIBS += -llog4cxx
    DEFINES += SAS_LOG4CXX_ENABLED
}

LIBS += -lpaho-mqtt3a
LIBS += -L../sasCore -lsasCore
LIBS += -L../sasJSON -lsasJSON

SOURCES += \
    mqttasync.cpp \
    mqttclient.cpp \
    mqttcomponent.cpp \
    mqttconnector.cpp \
    mqttinterface.cpp \
    mqttconnectionoptions.cpp \
    mqttconnectorfactrory.cpp

HEADERS += \
    include/sasMQTT/mqttasync.h \
    include/sasMQTT/mqttclient.h \
    include/sasMQTT/mqttconnectionoptions.h \
    mqttconnector.h \
    mqttinterface.h \
    include/sasMQTT/config.h \
    threading.h \
    mqttconnectorfactrory.h
