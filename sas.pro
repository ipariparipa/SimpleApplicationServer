
!include("user.pri") { }

TEMPLATE = subdirs

SUBDIRS += \
    sasCore \
    sasBasics \
    sas \
    sasTCLTools\
    sasClient \
    sasBypass \
    sasSQL \
    test \

sasBasics.depends = sasCore
sas.depends = sasCore sasBasics
sasTCLTools.depends = sasCore
sasClient.depends = sasCore sasTCLTools sasBasics
sasBypass.depends = sasCore
sasSQL.depends = sasCore
test.depends = sasCore sasBasics sasSQL

CONFIG(SAS_ALL) {
    CONFIG += \
          SAS_JSON \
          SAS_MYSQL \
          SAS_ORACLE \
          SAS_ODBC \
          SAS_SQLCLIENT \
          SAS_MQTT \
          SAS_CORBA \
          SAS_TCL \
          SAS_PIDL \
          SAS_HTTP
}

CONFIG(SAS_PIDL) {
    SUBDIRS += \
        sasPIDL \
        pidlsas \

    sasPIDL.depends = sasCore
    pidlsas.depends = sasCore sasBasics sasJSON sasPIDL

    CONFIG += SAS_JSON
}

CONFIG(SAS_MQTT) {
    SUBDIRS += \
        sasMQTT \

    sasMQTT.depends = sasCore sasJSON

    CONFIG += SAS_JSON
}

CONFIG(SAS_HTTP) {
    SUBDIRS += \
        sasHTTP \

    sasHTTP.depends = sasCore sasJSON
    CONFIG += SAS_JSON
}

CONFIG(SAS_JSON) {
    SUBDIRS += \
        sasJSON \

    sasJSON.depends = sasCore
}

CONFIG(SAS_MYSQL) {
    SUBDIRS += \
        sasMySQL \

    sasMySQL.depends = sasCore sasSQL
}

CONFIG(SAS_ODBC) {
    SUBDIRS += \
        sasODBC \

    sasODBC.depends = sasCore sasSQL
}

CONFIG(SAS_ORACLE) {
    SUBDIRS += \
        sasOracle \

    sasOracle.depends = sasCore sasSQL
}

CONFIG(SAS_SQLCLIENT) {
    SUBDIRS += \
        sasSQLClient \

    sasSQLClient.depends = sasCore sasSQL sasTCL sasTCLTools
    CONFIG += SAS_TCL
}

CONFIG(SAS_CORBA) {
    SUBDIRS += \
        sasCorba \

    sasCorba.depends = sasCore
}

CONFIG(SAS_TCL) {
    SUBDIRS += \
        sasTCL \

    sasTCL.depends = sasCore sasTCLTools
}

