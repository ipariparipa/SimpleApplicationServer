DUMMY_FILE = generated/pidladmin.json
INTERFACE_INPUT = pidljob.json

# Generating interface files
pidlCompile.input = INTERFACE_INPUT
pidlCompile.output = $$DUMMY_FILE
pidlCompile.variable_out = DISTFILES
pidlCompile.commands = if [ -z $$PIDL_BUILD_PATH ]; then \
                           cd $$_PRO_FILE_PWD_; \
                           ./prebuild.sh -file ${QMAKE_FILE_IN}; \
                       else \
                           export PIDLDIR="$$PIDL_BUILD_PATH"; \
                           cd $$_PRO_FILE_PWD_; \
                           ./prebuild.sh -file ${QMAKE_FILE_IN}; \
                       fi;
QMAKE_EXTRA_COMPILERS += pidlCompile

# Inform qmake that we are generatig these files.
pidladminHeader.input = DUMMY_FILE
pidladminHeader.output = $$_PRO_FILE_PWD_/generated/pidladmin.h
pidladminHeader.variable_out = HEADERS
pidladminHeader.commands = @true
pidladminHeader.depends = $$DUMMY_FILE
QMAKE_EXTRA_COMPILERS += pidladminHeader

pidladminSource.input = DUMMY_FILE
pidladminSource.output = $$_PRO_FILE_PWD_/generated/pidladmin.cpp
pidladminSource.variable_out = SOURCES
pidladminSource.commands = @true
pidladminSource.depends = $$_PRO_FILE_PWD_/generated/pidladmin.h
QMAKE_EXTRA_COMPILERS += pidladminSource
