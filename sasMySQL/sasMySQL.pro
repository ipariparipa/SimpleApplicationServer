include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasSQL/include

LIBS += -llog4cxx
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
