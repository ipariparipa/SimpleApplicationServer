include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -llog4cxx

SOURCES += \
    application.cpp \
    component.cpp \
    configreader.cpp \
    connector.cpp \
    controlledthread.cpp \
    componentloader.cpp \
    errorcollector.cpp \
    interfacemanager.cpp \
    invoker.cpp \
    libraryloader.cpp \
    logging.cpp \
    module.cpp \
    object.cpp \
    objectregistry.cpp \
    sas.cpp \
    session.cpp \
    sessionmanager.cpp \
    thread.cpp \
    timerthread.cpp \
    tools.cpp \
    watchdog.cpp \
	uniqueobjectmanager.cpp

HEADERS += \
    include/sasCore/application.h \
    include/sasCore/basictypes.h \
    include/sasCore/component.h \
    include/sasCore/componentloader.h \
    include/sasCore/config.h \
    include/sasCore/configreader.h \
    include/sasCore/connector.h \
    include/sasCore/controlledthread.h \
    include/sasCore/defines.h \
    include/sasCore/errorcodes.h \
    include/sasCore/errorcollector.h \
    include/sasCore/init.h \
    include/sasCore/interface.h \
    include/sasCore/interfacemanager.h \
    include/sasCore/invoker.h \
    include/sasCore/libraryloader.h \
    include/sasCore/logging.h \
    include/sasCore/module.h \
    include/sasCore/object.h \
    include/sasCore/objectregistry.h \
    include/sasCore/session.h \
    include/sasCore/sessionmanager.h \
    include/sasCore/thread.h \
    include/sasCore/timerthread.h \
    include/sasCore/tools.h \
    include/sasCore/watchdog.h \
    include/sasCore/platform.h \
	include/sasCore/uniqueobjectmanager.h



