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

LIBS += -L../sasCore -lsasCore

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
