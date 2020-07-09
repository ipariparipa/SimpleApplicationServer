include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasBasics/include
INCLUDEPATH += ../sasTCLTools/include

LIBS += -llog4cxx
LIBS += -L../sasCore -lsasCore
LIBS += -L../sasBasics -lsasBasics
LIBS += -L../sasTCLTools -lsasTCLTools

SOURCES += \
    sasapplication.cpp \
    sasbinarydata.cpp \
    sasconector.cpp \
    sasconfigreader.cpp \
    saserrorcollector.cpp \
    sasobject.cpp \
    sasobjectregistry.cpp \
    sasserver.cpp \
    sasstring.cpp \
    sastcldatawriter.cpp \
    sastcllist.cpp \
    sastclreader.cpp

HEADERS += \
    include/sasClient/config.h \
    include/sasClient/sasapplication.h \
    include/sasClient/sasbasetypes.h \
    include/sasClient/sasbinarydata.h \
    include/sasClient/sasconfigreader.h \
    include/sasClient/sasconnector.h \
    include/sasClient/saserrorcollector.h \
    include/sasClient/sasobject.h \
    include/sasClient/sasobjectregistry.h \
    include/sasClient/sasserver.h \
    include/sasClient/sasstring.h \
    include/sasClient/sastcldatareader.h \
    include/sasClient/sastcldatawriter.h \
    include/sasClient/sastcllist.h
