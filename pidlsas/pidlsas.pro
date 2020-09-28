PROJ_ROOT = $$_PRO_FILE_PWD_/../..
PROJ_OUT_ROOT = $$OUT_PWD/../..
include(../global.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../sasCore/include
INCLUDEPATH += ../sasBasics/include
INCLUDEPATH += ../sasJSON/include
INCLUDEPATH += ../sasPIDL/include

LIBS += -llog4cxx
LIBS += -L../sasCore -lsasCore
LIBS += -L../sasBasics -lsasBasics
LIBS += -L../sasJSON -lsasJSON
LIBS += -L../sasPIDL -lsasPIDL

defined(PIDL_BUILD_PATH, var) {
    LIBS += -L$$PIDL_BUILD_PATH/pidlBackend
    LIBS += -L$$PIDL_BUILD_PATH/pidlCore
}
defined(PIDL_PROJ_PATH, var) {
    INCLUDEPATH += $$PIDL_PROJ_PATH/pidlCore/include
    INCLUDEPATH += $$PIDL_PROJ_PATH/pidlBackend/include
}

LIBS += -lpidlBackend -lpidlCore

include("pidl-build.pri")

SOURCES += \
    main.cpp \

DISTFILES += \
    $$INTERFACE_HEADER_INPUT
    $$INTERFACE_SOURCE_INPUT
