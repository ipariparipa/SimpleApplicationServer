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
    sasgetpidl

sasBasics.depends = sasCore
sas.depends = sasCore sasBasics
sasTCLTools.depends = sasCore
sasClient.depends = sasCore sasTCLTools sasBasics
sasBypass = sasCore
sasJSON = sasCore
sasSQL = sasCore
sasMySQL = sasSQL sasCore
sasODBC = sasSQL sasCore
sasOracle = sasSQL sasCore
sasTCL = sasCore sasTCLTools
sasPIDL = sasCore
sasgetpidl = sasCore sasPIDL
