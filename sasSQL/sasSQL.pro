include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include

LIBS += -llog4cxx
LIBS += -L../sasCore -lsasCore

SOURCES += \
    sqlconnector.cpp \
    sqldatetime.cpp \
    sqlresult.cpp \
    sqlvariant.cpp

HEADERS += \
    include/sasSQL/config.h \
    include/sasSQL/errorcodes.h \
    include/sasSQL/sqlconnector.h \
    include/sasSQL/sqldatetime.h \
    include/sasSQL/sqlresult.h \
    include/sasSQL/sqlstatement.h \
    include/sasSQL/sqlvariant.h
