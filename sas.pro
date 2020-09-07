TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    sasCore \
    sasBasics \
    sas \
    sasTCLTools\
    sasClient \
    sasBypass \
    sasJSON \
    sasSQL \
    sasSQLClient \
    sasMySQL \
    sasODBC \
    sasOracle \
    sasHTTP \
    sasMQTT \
    sasCorba \
    sasTCL \
    sasPIDL \
    pidlsas \
    test

sasBasics.depends = sasCore
sas.depends = sasCore sasBasics
sasTCLTools.depends = sasCore
sasClient.depends = sasCore sasTCLTools sasBasics
sasBypass.depends = sasCore
sasJSON.depends = sasCore
sasSQL.depends = sasCore
sasMySQL.depends = sasSQL sasCore
sasODBC.depends = sasSQL sasCore
sasOracle.depends = sasSQL sasCore
sasTCL.depends = sasCore sasTCLTools
sasPIDL.depends = sasCore
pidlsas.depends = sasCore sasPIDL
test.depends = sasCore sasBasics sasSQL
