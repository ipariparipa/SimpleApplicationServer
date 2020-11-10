include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasJSON/include

CONFIG(SAS_LOG4CXX_ENABLED) {
    LIBS += -llog4cxx
    DEFINES += SAS_LOG4CXX_ENABLED
}

LIBS += -lneon
LIBS += -lmicrohttpd
LIBS += -L../sasCore -lsasCore
LIBS += -L../sasJSON -lsasJSON

SOURCES += \
    httpcomponent.cpp \
    httpconnector.cpp \
    httpinterface.cpp \
    httpconnectorfactory.cpp

HEADERS += \
    config.h \
    httpcommon.h \
    httpconnector.h \
    httpinterface.h \
    httpconnectorfactory.h
