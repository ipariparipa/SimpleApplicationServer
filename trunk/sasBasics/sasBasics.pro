include(../common.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include

LIBS += -llog4cxx

SOURCES += \
    envconfigreader.cpp \
    logging.cpp \
    server.cpp

HEADERS += \
    include/sasBasics/config.h \
    include/sasBasics/envconfigreader.h \
    include/sasBasics/errorcodes.h \
    include/sasBasics/logging.h \
    include/sasBasics/server.h \
    include/sasBasics/streamerrorcollector.h
