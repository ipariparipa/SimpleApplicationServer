include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasSQL/include
INCLUDEPATH += ../sasTCL/include
INCLUDEPATH += ../sasTCLTools/include

CONFIG(SAS_LOG4CXX_ENABLED) {
    LIBS += -llog4cxx
    DEFINES += SAS_LOG4CXX_ENABLED
}

LIBS += -L../sasCore -lsasCore
LIBS += -L../sasSQL -lsasSQL
LIBS += -L../sasTCL -lsasTCL
LIBS += -L../sasTCLTools -lsasTCLTools

SOURCES += \
    sc_module.cpp \
    sc_component.cpp

HEADERS += \
    sc_module.h \
    config.h
