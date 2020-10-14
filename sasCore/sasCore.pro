include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(SAS_LOG4CXX_ENABLED) {
    LIBS += -llog4cxx
    DEFINES += SAS_LOG4CXX_ENABLED
}

LIBS += -ldl -lpthread

TARGET_FILE = $$_PRO_FILE_PWD_/include/sasCore/platform.h
unix {
    TEMPLATE_FILE = $$_PRO_FILE_PWD_/include/sasCore/_platform_linux.h_
}

win32 {
    TEMPLATE_FILE = $_PRO_FILE_PWD_/include/sasCore/_platform_win.h_
}

win64 {
    TEMPLATE_FILE = $_PRO_FILE_PWD_/include/sasCore/_platform_win.h_
}

platform.input = TEMPLATE_FILE
platform.output = $$TARGET_FILE
platform.variable_out = HEADERS
platform.commands = cp $$TEMPLATE_FILE $$TARGET_FILE
QMAKE_EXTRA_COMPILERS += platform

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
	uniqueobjectmanager.cpp \
    notifier.cpp \
    timelinethread.cpp \
    threadpool.cpp

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
	include/sasCore/uniqueobjectmanager.h \
    include/sasCore/notifier.h \
    include/sasCore/timelinethread.h \
    include/sasCore/threadpool.h



