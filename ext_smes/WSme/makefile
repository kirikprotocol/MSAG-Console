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

DB_OCI_LIB:=$(SMSC_BUILDDIR)/bin/db/oci/libdb_oci.so
SME_BUILDDIR:=$(SMSC_BUILDDIR)/ext_smes/WSmeTmp


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
	@cp -f service $(SME_BUILDDIR)/bin
	@cp -f $(SMSC_BUILDDIR)/bin/wsme/WSme $(SME_BUILDDIR)/bin
	@cp -f $(SMSC_BUILDDIR)/jars/wsme.jar  $(SME_BUILDDIR)/lib/wsme.jar
	@cp -f log4cpp.properties $(SME_BUILDDIR)/log4cpp.properties
	@cp -f ../../config/wsme/config.xml $(SME_BUILDDIR)/conf/config.xml
	@cp -f ../../config/configuration.dtd $(SME_BUILDDIR)/conf/configuration.dtd
	@cp -f ../../config/command.dtd $(SME_BUILDDIR)/conf/command.dtd
	@cp -f ../../config/response.dtd $(SME_BUILDDIR)/conf/response.dtd
	@cp -f jsp/*.* $(SME_BUILDDIR)/jsp
	@cp -f jsp/inc/*.* $(SME_BUILDDIR)/jsp/inc
	@cp -f config.xml $(SME_BUILDDIR)

pack:
	@echo zipping archive
#	@cd $(SME_BUILDDIR); zip -9 -r $(SMSC_BUILDDIR)/ext_smes/dbsme bin conf jsp lib config.xml
	@jar -cfM $(SMSC_BUILDDIR)/ext_smes/wsme.war -C $(SME_BUILDDIR) .

clean:
	@echo Clean
	@${MAKE} -C java clean
	@rm -fr $(SME_BUILDDIR)
	@rm -f $(SMSC_BUILDDIR)/ext_smes/wsme.*
