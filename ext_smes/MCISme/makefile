SME_SRCDIR:=$(PWD)

ifndef SMSC_SRCDIR
#$(warning SMSC_SRCDIR is not defined)
SMSC_SRCDIR:=$(PWD)
$(warning set SMSC_SRCDIR = $(SMSC_SRCDIR))
export SMSC_SRCDIR
endif

ifndef SMSC_BUILDDIR
#$(warning SMSC_BUILDDIR is not defined)
SMSC_BUILDDIR:=$(HOME)/build/smsc
$(warning set SMSC_BUILDDIR = $(SMSC_BUILDDIR))
export SMSC_BUILDDIR
endif

SME_BUILDDIR:=$(SMSC_BUILDDIR)/ext_smes/MCISmeTmp
DB_OCI_LIB:=$(SMSC_BUILDDIR)/bin/db/oci/libdb_oci.so
DB_OCI_EXC:=$(SMSC_BUILDDIR)/bin/db/exceptions/libdb_exc.so
LOGGER_LIB:=$(SMSC_BUILDDIR)/bin/liblogger.so


default: compile pack

rebuild: clean compile pack


compile:
	@echo Compile
	@${MAKE} -C java compile
	@echo create package
	@mkdir -p $(SME_BUILDDIR)/bin
	@mkdir -p $(SME_BUILDDIR)/conf
	@mkdir -p $(SME_BUILDDIR)/jsp
	@mkdir -p $(SME_BUILDDIR)/jsp/inc
	@mkdir -p $(SME_BUILDDIR)/lib
	@cp -f service $(SME_BUILDDIR)/bin
	@cp -f  $(SMSC_BUILDDIR)/bin/mcisme/MCISme $(SME_BUILDDIR)/bin/
	@cp -f log4cpp.properties $(SME_BUILDDIR)/log4cpp.mcisme
	@cp -f $(SMSC_BUILDDIR)/jars/mcisme.jar  $(SME_BUILDDIR)/lib/mcisme.jar
	@cp -f $(DB_OCI_LIB) $(SME_BUILDDIR)/lib/libdb_oci.so
	@cp -f $(DB_OCI_EXC) $(SME_BUILDDIR)/lib/libdb_exc.so
	@cp -f $(LOGGER_LIB) $(SME_BUILDDIR)/liblogger.so
	@cp -f ../../config/mcisme/config.xml $(SME_BUILDDIR)/conf/config.xml
	@cp -f ../../config/configuration.dtd $(SME_BUILDDIR)/conf/configuration.dtd
	@cp -f ../../config/command.dtd $(SME_BUILDDIR)/conf/command.dtd
	@cp -f ../../config/response.dtd $(SME_BUILDDIR)/conf/response.dtd
	@cp -f jsp/*.* $(SME_BUILDDIR)/jsp
	@cp -f jsp/inc/*.* $(SME_BUILDDIR)/jsp/inc
	@cp -f config.xml $(SME_BUILDDIR)

pack:
	@echo zipping archive
	@jar -cfM $(SMSC_BUILDDIR)/ext_smes/mcisme.war -C $(SME_BUILDDIR) .

clean:
	@echo Clean
	@${MAKE} -C java clean
	@rm -fr $(SME_BUILDDIR)
	@rm -f $(SMSC_BUILDDIR)/ext_smes/mcisme.*
