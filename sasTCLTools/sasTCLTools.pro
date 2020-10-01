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

HEADERS += \
    include/sasTCLTools/config.h \
    include/sasTCLTools/errorcodes.h \
    include/sasTCLTools/tcldatareader.h \
    include/sasTCLTools/tcldatawriter.h \
    include/sasTCLTools/tcllist.h

SOURCES += \
    tcldatareader.cpp \
    tcldatawriter.cpp \
    tcllist.cpp
