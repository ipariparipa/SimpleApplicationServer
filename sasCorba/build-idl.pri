DUMMY_FILE = generated/corbadummy
INTERFACE_INPUT = corbasas.idl

idlCompile.input = INTERFACE_INPUT
idlCompile.output = $$DUMMY_FILE
idlCompile.variable_out = DISTFILES
idlCompile.commands =  cd $$_PRO_FILE_PWD_; \
                       omniidl -bcxx -Cgenerated ${QMAKE_FILE_IN}; \
                       touch $$DUMMY_FILE;
QMAKE_EXTRA_COMPILERS += idlCompile

corbaSASHeader.input = DUMMY_FILE
corbaSASHeader.output = $$_PRO_FILE_PWD_/generated/corbasas.hh
corbaSASHeader.variable_out = HEADERS
corbaSASHeader.commands = @true
corbaSASHeader.depends = $$DUMMY_FILE
QMAKE_EXTRA_COMPILERS += corbaSASHeader

corbaSASSource.input = DUMMY_FILE
corbaSASSource.output = $$_PRO_FILE_PWD_/generated/corbasasSK.cc
corbaSASSource.variable_out = SOURCES
corbaSASSource.commands = @true
corbaSASSource.depends = $$_PRO_FILE_PWD_/generated/corbasas.hh
QMAKE_EXTRA_COMPILERS += corbaSASSource 
