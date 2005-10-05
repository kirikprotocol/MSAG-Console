#
# $Id$
#
#
#

# to compile with sun compiler, use CXX=CC
ifndef CXX
CXX:=CC
endif
export CXX

ifeq ($(CXX),CC)
AR:=/usr/ccs/bin/ar
export AR
SHARED_OPT:=-G
else
SHARED_OPT:=-shared
endif

SSL_DIR:=/usr/local/ssl
ifeq ($(CXX),CC)
ifeq ($(SNMP),YES)
SSL_LIB:=-lcrypto -lssl
else
SSL_LIB:=$(SSL_DIR)/lib/libcrypto.a $(SSL_DIR)/lib/libssl.a
endif
else
SSL_LIB:=-lcrypto -lssl
endif

XERCES_LIB:=-lxerces-c
export XERCES_LIB
export SHARED_OPT
export SSL_LIB

ifndef XTARGET
XTARGET := -xtarget=ultra3 -xarch=v8plus
endif

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
CVSROOT:=:pserver:$(USER)@cvs:/cvsroot
$(warning set CVSROOT = $(CVSROOT))
export CVSROOT
endif

OS=$(shell uname)

ifeq ($(OS),Linux)
export SOCKETLIB=
else
export SOCKETLIB=-lsocket
endif


SS7_STACK_HOME := /opt/EABss7009
export SS7_STACK_HOME

SS7_LIBRARY_NAME := einss7_r
export SS7_LIBRARY_NAME

CAMMON_INCLUDES = -I$(SMSC_SRCDIR) -I/usr/local/include -I$(SSL_DIR)/include -I$(SS7_STACK_HOME)/src -DEINSS7_THREADSAFE=1 -DMAP_R12

CFLAGS := $(CAMMON_INCLUDES) -D_REENTERANT -D_REENTRANT -D_FILE_OFFSET_BITS=64
CXXFLAGS := $(CAMMON_INCLUDES) -D_REENTERANT -D_REENTRANT -D_FILE_OFFSET_BITS=64
ifeq ($(CXX),CC)
COMPFLAGS:=-features=extensions $(XTARGET) -mt -D_POSIX_PTHREAD_SEMANTICS
CXXFLAGS := $(CXXFLAGS) -library=stlport4
endif
ifeq ($(CXX),CC)
LDFLAGS := -zrescan -L$(SMSC_BUILDDIR)/lib -L/usr/local/lib/CC -L/usr/local/lib -L$(SSL_DIR)/lib
else
LDFLAGS := -L$(SMSC_BUILDDIR)/lib -L/usr/local/lib -L$(SSL_DIR)/lib
endif

ifneq ($(TRACE),YES)
CFLAGS := $(CFLAGS) -DDISABLE_TRACING
CXXFLAGS := $(CXXFLAGS) -DDISABLE_TRACING
endif

ifeq ($(LEAKTRACE),YES)
LDFLAGS := $(LDFLAGS) -lutil-leaktracing
EXTRADEPS:= $(SMSC_BUILDDIR)/lib/libutil-leaktracing.a
export EXTRADEPS
DEBUG := YES
endif

ifeq ($(DISABLE_SMPP_CHECKS),YES)
CXXFLAGS := $(CXXFLAGS) -DDISABLE_SMPP_CHECKS
endif

ifeq ($(LEAKTRACE),SIMPLE)
LDFLAGS := $(LDFLAGS) -lcore-threads -lutil-leaktracing-simple -lcore-threads
DEBUG := YES
endif

ifeq ($(NOMAPPROXY),YES)
CXXFLAGS := $(CXXFLAGS) -DNOMAPPROXY
endif

ifeq ($(SNMP),YES)
CXXFLAGS := $(CXXFLAGS) -DSNMP
SNMP_LIBS := -lnetsnmpagent -lnetsnmphelpers -lnetsnmpmibs -lnetsnmp -lkstat -lsnmp
export SNMP_LIBS
endif

ifdef WARNINGS
ifeq ($(CXX),CC)
COMPFLAGS := $(COMPFLAGS) +w
else
COMPFLAGS := $(COMPFLAGS) -Wall
endif
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
ifeq ($(CXX),CC)
CXXFLAGS := -g $(CXXFLAGS) -DDISABLE_ANY_CHECKS
CFLAGS := -g $(CFLAGS) -DDISABLE_ANY_CHECKS
else
CFLAGS := $(CFLAGS) -O2 -finline-limit=2048 -DDISABLE_ANY_CHECKS
CXXFLAGS := $(CXXFLAGS) -O2 -finline-limit=2048 -DDISABLE_ANY_CHECKS
endif
endif
endif
endif

ifdef ASSERT
CFLAGS := $(CFLAGS) -DASSERT_THROW_IF_FAIL
CXXFLAGS := $(CFLAGS) -DASSERT_THROW_IF_FAIL
endif

#ifneq ($(NOMAP),YES)
#endif

ifeq ($(HOSTTYPE),sparc)
CXXFLAGS := $(CXXFLAGS) -DSPARC
CFLAGS := $(CFLAGS) -DSPARC
endif


CXXFLAGS := $(CXXFLAGS) -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/rdbms/public -I$(ORACLE_HOME)/network/public
CFLAGS := $(CFLAGS) -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/rdbms/public -I$(ORACLE_HOME)/network/public

ifneq ("$(NOMAP)","YES")

MTYPE:=$(shell perl -e '$$x=$$ENV{MACHTYPE};$$x=~s/[\d.]//g;print $$x;')

ifeq ("$(MTYPE)","sparc-sun-solaris")
MAP_LIBRARIES := -L$(SS7_STACK_HOME)/lib -l$(SS7_LIBRARY_NAME) -lsystem-mapio
CXXFLAGS := $(CXXFLAGS) -DUSE_MAP
endif
endif

export CXXFLAGS
export CFLAGS
export LDFLAGS
export COMPFLAGS
export MAP_LIBRARIES

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
	    set -e; for i in $$XXXX; do $(MAKE) -s $$i.rbuild ; done; \
	fi

ubuild: .update .build
	@echo >> /dev/null

clean:
	@echo "clean all"
	@if [ -n "$(MODULES)" ]; then \
	    XXXX="$(MODULES)"; \
	    set -e; for i in $$XXXX; do $(MAKE) -s $$i.rclean ; done; \
	fi

deps:
	@echo "build dependencies"
	@if [ -n "$(MODULES)" ]; then \
	    XXXX="$(MODULES)"; \
	    set -e; for i in $$XXXX; do $(MAKE) -s $$i.deps ; done; \
	fi

$(SMSC_BUILDDIR): $(SMSC_BUILDDIR_LIST)

$(SMSC_BUILDDIR_LIST):
	@mkdir -p $(SMSC_BUILDDIR_LIST)

$(MODULES) $(foreach x,$(MODULES),$(x).%): __.empty.__ $(SMSC_BUILDDIR_LIST)
override _modules = $(subst ., ,$@)
override _module = $(firstword $(_modules))
override _subtarget = $(if $(word 2, $(_modules)), $(patsubst $(_module).%,%,$@), rbuild )
	@if [ -d $(_module) ]; then \
	    cd $(_module); $(MAKE) -s $(_subtarget); \
	else \
	    echo "'$(_module)' is not module"; exit 1;\
	fi

webapp.newbuild:
	@cd ../webapps/smsc;perl $(SMSC_SRCDIR)/conf/newbuild.pl webapp
