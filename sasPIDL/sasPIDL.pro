include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include

LIBS += -llog4cxx
LIBS += -L../sasCore -lsasCore

defined(PIDL_PROJ_PATH, var) {
    LIBS += -L../../$$PIDL_PROJ_PATH/pidlCore
    INCLUDEPATH += ../../$$PIDL_PROJ_PATH/pidlCore/include
}

LIBS += -lpidlCore

SOURCES += \
    errorcollector.cpp \
    pidljsonhelper.cpp

HEADERS += \
    include/sasPIDL/config.h \
    include/sasPIDL/errorcollector.h \
    include/sasPIDL/pidljsonclient.h \
    include/sasPIDL/pidljsonhelper.h \
    include/sasPIDL/pidljsoninvoker.h
