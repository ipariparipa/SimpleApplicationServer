include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasSQL/include

CONFIG(SAS_LOG4CXX_ENABLED) {
    LIBS += -llog4cxx
    DEFINES += SAS_LOG4CXX_ENABLED
}

LIBS += -lodpic
LIBS += -ldl
LIBS += -L../sasCore -lsasCore
LIBS += -L../sasSQL -lsasSQL

SOURCES += \
    oracomponent.cpp \
    oraconnector.cpp \
    orastatement.cpp \
    oratools.cpp

HEADERS += \
    config.h \
    oraconnector.h \
    orastatement.h \
    oratools.h
