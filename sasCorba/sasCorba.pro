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

LIBS += -lomniORB4
LIBS += -L../sasCore -lsasCore

include("build-idl.pri")

SOURCES += \
    corbacomponent.cpp \
    corbaconnector.cpp \
    corbainterface.cpp \
    tools.cpp \
    corbaconnectorfactory.cpp

HEADERS += \
    config.h \
    corbaconnector.h \
    corbainterface.h \
    tools.h \
    corbaconnectorfactory.h

DISTFILES += \
    corbasas.idl
