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

ifndef CVSROOT
#$(warning CVSROOT is not defined)
CVSROOT:=:pserver:$(USER)@src.novosoft.ru:/cvsroot/main
$(warning set CVSROOT = $(CVSROOT))
export CVSROOT
endif

CFLAGS= -I$(SMSC_SRCDIR)
CXXFLAGS= -I$(SMSC_SRCDIR)
LDFLAGS= -L$(SMSC_BUILDDIR)/lib

ifdef DEBUG
CFLAGS=$(CFLAGS) -g
elif OPTI
CFLAGS=$(CFLAGS) -O2
endif





all:
	@echo "?"

commit:
	@echo "commit cources into CVS"
	cvs -ecat commit
	 
update:
	@echo "updating sources from CVS"
	cvs update -d 
	
build:	$(SMSC_BUILDDIR)
	@echo "building SMSC"
	
ubuild:	update build



$(SMSC_BUILDDIR):
	mkdir -p $(SMSC_BUILDDIR)
	mkdir -p $(SMSC_BUILDDIR)\lib
	mkdir -p $(SMSC_BUILDDIR)\bin
	mkdir -p $(SMSC_BUILDDIR)\etc
	mkdir -p $(SMSC_BUILDDIR)\doc
	
%:
	echo "?????"	
	
