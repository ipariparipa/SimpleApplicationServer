include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasTCLTools/include

CONFIG(SAS_LOG4CXX_ENABLED) {
    LIBS += -llog4cxx
    DEFINES += SAS_LOG4CXX_ENABLED
}

LIBS += -L../sasCore -lsasCore
LIBS += -L../sasTCLTools -lsasTCLTools

HEADERS += \
    include/sasTCL/config.h \
    include/sasTCL/errorcodes.h \
    include/sasTCL/tclconfigreader.h \
    include/sasTCL/tclerrorcollector.h \
    include/sasTCL/tclexecutor.h \
    include/sasTCL/tclinterpinitilizer.h \
    include/sasTCL/tclinvoker.h \
    include/sasTCL/tcllisthandler.h
    include/sasTCL/tclobjectref.h


SOURCES += \
    tclconfigreader.cpp \
    tclerrorcollector.cpp \
    tclexecutor.cpp \
    tclinvoker.cpp \
    tcllisthandler.cpp
    tclobjectref.cpp

