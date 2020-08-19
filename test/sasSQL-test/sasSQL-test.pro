
include("../../global.pri")

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
#TARGET =

#QMAKE_CXXFLAGS += -std=c++17

SOURCES += main.cpp \
           datetime_test.cpp

HEADERS += \
           datetime_test.h

LIBS += -L../../sasCore -lsasCore
LIBS += -L../../sasBasics -lsasBasics
LIBS += -L../../sasSQL -lsasSQL
INCLUDEPATH += ../../sasCore/include
INCLUDEPATH += ../../sasBasics/include
INCLUDEPATH += ../../sasSQL/include

LIBS += -lcppunit -lcrypto -lpthread
