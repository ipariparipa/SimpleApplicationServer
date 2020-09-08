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

defined(PIDL_BUILD_PATH, var) {
    system( echo "Generating interface files..."; \
            export PIDLDIR="$$PIDL_BUILD_PATH"; \
            cd $$_PRO_FILE_PWD_; \
            rm generated/*; \
            ./prebuild.sh -file ./pidljob.json )
} else {
    system( echo "Generating interface files..."; \
            cd $$_PRO_FILE_PWD_; \
            rm generated/*; \
            ./prebuild.sh -file ./pidljob.json )
}

SOURCES += \
    main.cpp \
    generated/pidladmin.cpp \

HEADERS += \
    generated/pidladmin.h \
