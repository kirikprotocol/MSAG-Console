SME_SRCDIR:=$(PWD)

ifndef SMSC_BUILDDIR
#$(warning SMSC_BUILDDIR is not defined)
SMSC_BUILDDIR:=$(HOME)/build/smsc
$(warning set SMSC_BUILDDIR = $(SMSC_BUILDDIR))
export SMSC_BUILDDIR
endif

DB_OCI_LIB:=$(SMSC_BUILDDIR)/bin/db/oci/libdb_oci.so
SME_BUILDDIR:=$(SMSC_BUILDDIR)/ext_smes/dbSmeTmp


default: compile pack

rebuild: clean compile pack


compile:
	@echo Compile
	@${MAKE} -C java compile
	@echo create package
	@mkdir -p $(SME_BUILDDIR)/bin
	@mkdir -p $(SME_BUILDDIR)/conf
	@mkdir -p $(SME_BUILDDIR)/lib
	@mkdir -p $(SME_BUILDDIR)/jsp
	@cp -f $(SMSC_BUILDDIR)/bin/dbsme/DBSme $(SME_BUILDDIR)/bin/service
	@cp -f $(SMSC_BUILDDIR)/jars/dbSme.jar  $(SME_BUILDDIR)/lib/dbSme.jar
	@cp -f $(DB_OCI_LIB) $(SME_BUILDDIR)/lib/libdb_oci.so
	@cp -f ../../config/dbsme/config.xml $(SME_BUILDDIR)/conf/config.xml
	@cp -f ../../config/configuration.dtd $(SME_BUILDDIR)/conf/configuration.dtd
	@cp -f jsp/*.* $(SME_BUILDDIR)/jsp
	@cp -f config.xml $(SME_BUILDDIR)

pack:
	@echo zipping archive
#	@cd $(SME_BUILDDIR); zip -9 -r $(SMSC_BUILDDIR)/ext_smes/dbsme bin conf jsp lib config.xml
	@jar -cfM $(SMSC_BUILDDIR)/ext_smes/dbsme.war -C $(SME_BUILDDIR) .

clean:
	@echo Clean
	@${MAKE} -C java clean
	@rm -fr $(SME_BUILDDIR)
	@rm -f $(SMSC_BUILDDIR)/ext_smes/dbsme.*
