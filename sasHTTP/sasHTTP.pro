include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasJSON/include

LIBS += -llog4cxx
LIBS += -lneon
LIBS += -lmicrohttpd
LIBS += -L../sasCore -lsasCore
LIBS += -L../sasJSON -lsasJSON

SOURCES += \
    httpcomponent.cpp \
    httpconnector.cpp \
    httpinterface.cpp

HEADERS += \
    config.h \
    httpcommon.h \
    httpconnector.h \
    httpinterface.h
