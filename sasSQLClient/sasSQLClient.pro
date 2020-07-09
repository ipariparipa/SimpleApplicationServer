include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasSQL/include
INCLUDEPATH += ../sasTCL/include
INCLUDEPATH += ../sasTCLTools/include

LIBS += -llog4cxx
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
