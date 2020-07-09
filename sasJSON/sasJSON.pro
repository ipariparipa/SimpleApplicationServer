include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include

LIBS += -llog4cxx
LIBS += -L../sasCore -lsasCore

SOURCES += \
    jsonconfigreader.cpp \
    jsondocumentreader.cpp \
    jsonerrorcollector.cpp \
    jsoninvoker.cpp \
    jsonreader.cpp

HEADERS += \
    include/sasJSON/config.h \
    include/sasJSON/errorcodes.h \
    include/sasJSON/jsonconfigreader.h \
    include/sasJSON/jsondocument.h \
    include/sasJSON/jsondocumentreader.h \
    include/sasJSON/jsonerrorcollector.h \
    include/sasJSON/jsoninvoker.h \
    include/sasJSON/jsonreader.h
