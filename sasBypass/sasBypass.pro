include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include

CONFIG(SAS_LOG4CXX_ENABLED) {
    LIBS += -llog4cxx
    DEFINES += SAS_LOG4CXX_ENABLED
}

LIBS += -L../sasCore -lsasCore

SOURCES += \
    bp_component.cpp \
    bypassmodule.cpp \
    loopbackconnector.cpp

HEADERS += \
    bypassmodule.h \
    config.h \
    loopbackconnector.h
