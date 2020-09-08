TEMPLATE = subdirs

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
sasSQLClient.depends = sasCore sasSQL sasTCL sasTCLTools
sasMySQL.depends = sasCore sasSQL
sasHTTP.depends = sasCore sasJSON
sasMQTT.depends = sasCore sasJSON
sasCorba.depends = sasCore
sasODBC.depends = sasCore sasSQL
sasOracle.depends = sasCore sasSQL
sasTCL.depends = sasCore sasTCLTools
sasPIDL.depends = sasCore
pidlsas.depends = sasCore sasBasics sasJSON sasPIDL
test.depends = sasCore sasBasics sasSQL
