include(../global.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasBasics/include

LIBS += -llog4cxx
LIBS += -L../sasCore -lsasCore
LIBS += -L../sasBasics -lsasBasics

SOURCES += \
    main.cpp \
    sasserver.cpp

HEADERS += \
    sasserver.h \
    version.h
