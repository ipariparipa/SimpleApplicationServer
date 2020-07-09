include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include

LIBS += -llog4cxx
LIBS += -L../sasCore -lsasCore

SOURCES += \
    bp_component.cpp \
    bypassmodule.cpp \
    loopbackconnector.cpp

HEADERS += \
    bypassmodule.h \
    config.h \
    loopbackconnector.h
