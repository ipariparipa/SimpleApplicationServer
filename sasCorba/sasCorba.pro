include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include

LIBS += -llog4cxx
LIBS += -lomniORB4
LIBS += -L../sasCore -lsasCore

defined(PIDL_BUILD_PATH, var) {
    system( echo "Generating interface files..."; \
            #rm generated/*; \
            ./idl.sh )
}

SOURCES += \
    corbacomponent.cpp \
    corbaconnector.cpp \
    corbainterface.cpp \
    tools.cpp \
    generated/corbasasSK.cc

HEADERS += \
    config.h \
    corbaconnector.h \
    corbainterface.h \
    tools.h \
    generated/corbasas.hh

DISTFILES += \
    corbasas.idl
