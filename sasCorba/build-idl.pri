INTERFACE_INPUT = corbasas.idl

corbaSASHeader.input = INTERFACE_INPUT
corbaSASHeader.output = $$_PRO_FILE_PWD_/generated/corbasas.hh
corbaSASHeader.variable_out = HEADERS
corbaSASHeader.commands =  cd $$_PRO_FILE_PWD_; \
                           omniidl -bcxx -Cgenerated ${QMAKE_FILE_IN};
corbaSASHeader.CONFIG = target_predeps
QMAKE_EXTRA_COMPILERS += corbaSASHeader

corbaSASSource.input = INTERFACE_INPUT
corbaSASSource.output = $$_PRO_FILE_PWD_/generated/corbasasSK.cc
corbaSASSource.variable_out = SOURCES
corbaSASSource.commands = cd $$_PRO_FILE_PWD_; \
                          omniidl -bcxx -Cgenerated ${QMAKE_FILE_IN};
corbaSASSource.depends = $$_PRO_FILE_PWD_/generated/corbasas.hh
QMAKE_EXTRA_COMPILERS += corbaSASSource 
