#
# $Id$
#
#
#

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

CAMMON_INCLUDES = -I$(SMSC_SRCDIR) -I/usr/local/include
CFLAGS := $(CAMMON_INCLUDES) -D_REENTERANT
CXXFLAGS := $(CAMMON_INCLUDES) -D_REENTERANT
LDFLAGS := -L$(SMSC_BUILDDIR)/lib

ifneq ($(TRACE),YES)
CFLAGS := $(CFLAGS) -DDISABLE_TRACING
CXXFLAGS := $(CXXFLAGS) -DDISABLE_TRACING
endif

ifeq ($(LEAKTRACE),YES)
LDFLAGS := $(LDFLAGS) -lcore-threads -lutil-leaktracing -lcore-threads
DEBUG := YES
endif

ifdef WARNINGS
CFLAGS := $(CFLAGS) -Wall
CXXFLAGS := $(CXXFLAGS) -Wall
endif

ifeq ($(DEBUG),YES)
CFLAGS := $(CFLAGS) -g -DSMSC_DEBUG
CXXFLAGS := $(CXXFLAGS) -g -DSMSC_DEBUG
export DEBUG
else
ifeq ($(COVER),YES)
CFLAGS := $(CFLAGS) -pg -g -a
CXXFLAGS := $(CXXFLAGS) -pg -g -a
else
ifdef OPTI
CFLAGS := $(CFLAGS) -O2 -finline-limit=16384 -DDISABLE_ANY_CHECKS
CXXFLAGS := $(CXXFLAGS) -O2 -finline-limit=16384 -DDISABLE_ANY_CHECKS
endif
endif
endif

ifdef ASSERT
CFLAGS := $(CFLAGS) -DASSERT_THROW_IF_FAIL
CXXFLAGS := $(CFLAGS) -DASSERT_THROW_IF_FAIL
endif

CXXFLAGS := $(CXXFLAGS) -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public
CFLAGS := $(CFLAGS) -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public

export CXXFLAGS
export CFLAGS
export LDFLAGS

MODULES := $(shell cat modules-list | awk '/^[^\#]./{print $$1}' )
SMSC_BUILDDIR_LIST = $(foreach x,lib bin obj etc doc,$(SMSC_BUILDDIR)/$(x))

#$(warning $(MODULES))

all:	build
	@echo >> /dev/null

__.empty.__:
	@echo >> /dev/null

commit:
	@echo "commit cources into CVS"
	cvs -ecat commit

update:
	@echo "updating sources from CVS"
	cvs update -d

docs:
	@echo Generating docs
	@mkdir -p $(SMSC_BUILDDIR)/doc/html
	@perl -e 'while(<>){s!\$$\(DOCSDIR\)!$(SMSC_BUILDDIR)/doc/html!;print;}' <Doxyfile.tmpl >Doxyfile
	@doxygen Doxyfile
	@rm Doxyfile

__build_message__:
	@echo "build in SMSC"

build: __build_message__ $(SMSC_BUILDDIR_LIST)
	@if [ -n "$(MODULES)" ]; then \
	    XXXX="$(MODULES)"; \
	    set -e; for i in $$XXXX; do make -s $$i.rbuild ; done; \
	fi

ubuild: .update .build
	@echo >> /dev/null

clean:
	@echo "clean all"
	@if [ -n "$(MODULES)" ]; then \
	    XXXX="$(MODULES)"; \
	    set -e; for i in $$XXXX; do make -s $$i.rclean ; done; \
	fi

deps:
	@echo "build dependencies"
	@if [ -n "$(MODULES)" ]; then \
	    XXXX="$(MODULES)"; \
	    set -e; for i in $$XXXX; do make -s $$i.deps ; done; \
	fi

$(SMSC_BUILDDIR): $(SMSC_BUILDDIR_LIST)

$(SMSC_BUILDDIR_LIST):
	@mkdir -p $(SMSC_BUILDDIR_LIST)

$(MODULES) $(foreach x,$(MODULES),$(x).%): __.empty.__ $(SMSC_BUILDDIR_LIST)
override _modules = $(subst ., ,$@)
override _module = $(firstword $(_modules))
override _subtarget = $(if $(word 2, $(_modules)), $(patsubst $(_module).%,%,$@), rbuild )
	@if [ -d $(_module) ]; then \
	    cd $(_module); make -s $(_subtarget); \
	else \
	    echo "'$(_module)' is not module"; exit 1;\
	fi
