include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasSQL/include

LIBS += -llog4cxx
LIBS += -lodbc
LIBS += -L../sasCore -lsasCore
LIBS += -L../sasSQL -lsasSQL

SOURCES += \
    odbccomponent.cpp \
    odbcconnector.cpp \
    odbctools.cpp \
    odbcstatement.cpp

HEADERS += \
    config.h \
    include_odbc.h \
    odbcconnector.h \
    odbcresult.h \
    odbctools.h \
    odbcstatement.h
