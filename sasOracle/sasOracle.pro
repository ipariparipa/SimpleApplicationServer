include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasSQL/include

LIBS += -llog4cxx
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
