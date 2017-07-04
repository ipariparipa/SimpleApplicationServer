include(../common.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include

LIBS += -llog4cxx

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
