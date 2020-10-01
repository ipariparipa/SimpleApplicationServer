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

LIBS += -lmysqlclient
LIBS += -L../sasCore -lsasCore
LIBS += -L../sasSQL -lsasSQL

SOURCES += \
    mysqlcomponent.cpp \
    mysqlconnector.cpp \
    mysqlresult.cpp \
    mysqlstatement.cpp

HEADERS += \
    mysqlconnector.h \
    mysqlresult.h \
    mysqlstatement.h \
    config.h
