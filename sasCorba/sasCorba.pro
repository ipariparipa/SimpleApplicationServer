include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include

LIBS += -llog4cxx
LIBS += -lomniORB4
LIBS += -L../sasCore -lsasCore

include("build-idl.pri")

SOURCES += \
    corbacomponent.cpp \
    corbaconnector.cpp \
    corbainterface.cpp \
    tools.cpp \

HEADERS += \
    config.h \
    corbaconnector.h \
    corbainterface.h \
    tools.h \

DISTFILES += \
    corbasas.idl
