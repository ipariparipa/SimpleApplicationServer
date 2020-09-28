# TODO
#defined(PIDL_BUILD_PATH, var) {
#    system( echo "Generating interface files..."; \
#            export PIDLDIR="$$PIDL_BUILD_PATH"; \
#            cd $$_PRO_FILE_PWD_; \
#            rm generated/*; \
#            ./prebuild.sh -file ./pidljob.json )
#} else {
#    system( echo "Generating interface files..."; \
#            cd $$_PRO_FILE_PWD_; \
#            rm generated/*; \
#            ./prebuild.sh -file ./pidljob.json )
#}

INTERFACE_HEADER_INPUT = pidljob_header.json
INTERFACE_SOURCE_INPUT = pidljob_source.json

pidladminHeader.input = INTERFACE_HEADER_INPUT
pidladminHeader.output = $$_PRO_FILE_PWD_/generated/pidladmin.h
pidladminHeader.variable_out = HEADERS
pidladminHeader.commands = export PIDLDIR="$$PIDL_BUILD_PATH"; \
                           cd $$_PRO_FILE_PWD_; \
                           ./prebuild.sh -file ${QMAKE_FILE_IN};
QMAKE_EXTRA_COMPILERS += pidladminHeader

pidladminSource.input = INTERFACE_SOURCE_INPUT
pidladminSource.output = $$_PRO_FILE_PWD_/generated/pidladmin.cpp
pidladminSource.variable_out = SOURCES
pidladminSource.commands = export PIDLDIR="$$PIDL_BUILD_PATH"; \
                           cd $$_PRO_FILE_PWD_; \
                           ./prebuild.sh -file ${QMAKE_FILE_IN};
pidladminSource.depends = $$_PRO_FILE_PWD_/generated/pidladmin.h
QMAKE_EXTRA_COMPILERS += pidladminSource
