#
# $Id$
#
#
#
# to compile with sun compiler, use CXX=CC

-include makefile.flags

ifeq ($(INMAN_CUSTOM_LINKAGE),YES)
-include inman/makefile.flags
endif

ifndef CXX
CXX:=CC
endif
export CXX

export CC=cc

ifndef WARNINGS
WARNINGS=YES
endif

export WARNINGS

ifeq ($(CXX),CC)
AR:=/usr/ccs/bin/ar
export AR
SHARED_OPT:=-G
else
SHARED_OPT:=-shared
endif

USRLOCLIB := /usr/local/lib
ifeq ($(B64),YES)
USRLOCLIB := /usr/local/lib/64
endif

ifdef ORACLE_HOME
ifndef ORACLE_LIB
ifeq ($(B64),YES)
  ORACLE_LIB = $(ORACLE_HOME)/lib
else
  ORACLE_LIB = $(ORACLE_HOME)/lib32
endif
 export ORACLE_LIB
endif
 ORACLE_INC = -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/rdbms/public -I$(ORACLE_HOME)/network/public
endif


ifeq ($(SFW),YES)
SSL_LIB_DIR:=/usr/sfw/lib
NETSNMP_LIB_DIR:=/usr/sfw/lib
ifeq ($(B64),YES)
SSL_LIB_DIR:=/usr/sfw/lib/64
NETSNMP_LIB_DIR:=/usr/sfw/lib/64
endif
else
SSL_LIB_DIR:=/usr/lib
NETSNMP_LIB_DIR:=/usr/lib
ifeq ($(B64),YES)
SSL_LIB_DIR:=/usr/lib/64
NETSNMP_LIB_DIR:=/usr/lib/64
endif
endif


ifeq ($(CXX),CC)
ifeq ($(SNMP),YES)
SSL_LIB:=-lcrypto -lssl
else
SSL_LIB:=$(SSL_LIB_DIR)/libcrypto.a $(SSL_LIB_DIR)/libssl.a
endif
else
SSL_LIB:=-lcrypto -lssl
endif

XERCES_LIB:=-lxerces-c
export XERCES_LIB
export SHARED_OPT
export SSL_LIB

ifndef XTARGET
ifeq ($(B64),YES)
XTARGET := -xtarget=ultra3 -xarch=v9 -xmemalign=8s -xcode=pic32
else
XTARGET := -xtarget=ultra3 -xarch=v8plus
endif
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

OS=$(shell uname)

ifeq ($(OS),Linux)
export SOCKETLIB=
else
export SOCKETLIB=-lsocket
endif

ifndef SS7_STACK_HOME
SS7_STACK_HOME := /opt/EABss7009/r4f_c
endif
export SS7_STACK_HOME

SS7_LIBRARY_NAME := einss7_r
ifeq ($(B64),YES)
SS7_LIBRARY_NAME := einss7_r_64
endif

export SS7_LIBRARY_NAME

COMMON_INCLUDES := -I$(SMSC_SRCDIR)
ifeq ($(USEHOME),YES)
COMMON_INCLUDES := $(COMMON_INCLUDES) -I$(HOME)/include
endif

ifeq ($(SFW),YES)
COMMON_INCLUDES := $(COMMON_INCLUDES) -I/usr/sfw/include -I/usr/local/include
else
COMMON_INCLUDES := $(COMMON_INCLUDES) -I/usr/local/include
endif
COMMON_INCLUDES := $(COMMON_INCLUDES) -I$(SS7_STACK_HOME)/src -DEINSS7_THREADSAFE=1 -DMAP_R12 -D__EXTENSIONS__ -I/usr/local/ssl/include

CFLAGS := $(CFLAGS) $(COMMON_INCLUDES) -D_REENTERANT -D_REENTRANT -D_FILE_OFFSET_BITS=64
CXXFLAGS := $(CXXFLAGS) $(COMMON_INCLUDES) -D_REENTERANT -D_REENTRANT -D_FILE_OFFSET_BITS=64
ifeq ($(CXX),CC)
COMPFLAGS:=-features=extensions -mt -D_POSIX_PTHREAD_SEMANTICS
ifeq ($(STDCXX),YES)
CXXFLAGS := $(CXXFLAGS) $(XTARGET) -library=%none -I/opt/stdcxx/include
else
CXXFLAGS := $(CXXFLAGS) $(XTARGET) -library=stlport4
endif
CFLAGS := $(CFLAGS) $(XTARGET)
endif
ifeq ($(CXX),CC)
LDFLAGS := $(LDFLAGS) $(XTARGET) -L$(SMSC_BUILDDIR)/bin -L$(SMSC_BUILDDIR)/lib -L$(USRLOCLIB) -L$(SSL_LIB_DIR) -L$(NETSNMP_LIB_DIR)
ifeq ($(STDCXX),YES)
LDFLAGS := $(LDFLAGS) -L/opt/stdcxx/lib -lstd -lthread
endif
else
LDFLAGS := $(LD_FLAGS) -L$(SMSC_BUILDDIR)/bin -L$(SMSC_BUILDDIR)/lib -L$(USRLOCLIB) -L$(SSL_LIB_DIR)
CXXFLAGS := $(CXXFLAGS) -fPIC
endif

ifeq ($(USEHOME),YES)
LDFLAGS := $(LDFLAGS) -L$(HOME)/lib
endif

ifneq ($(TRACE),YES)
CFLAGS := $(CFLAGS) -DDISABLE_TRACING
CXXFLAGS := $(CXXFLAGS) -DDISABLE_TRACING
endif

ifeq ($(LEAKTRACE),YES)
CXXFLAGS:= $(CXXFLAGS) -DLEAKTRACE
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

ifeq ($(INTHASHUSAGECHECKING),YES)
CXXFLAGS := $(CXXFLAGS) -DINTHASH_USAGE_CHECKING
endif

ifeq ($(SNMP),YES)
ifeq ($(CXX),CC)
SNMP_LIBS_CORE = -lnetsnmpagent -lnetsnmphelpers -lnetsnmpmibs -lnetsnmp -lkstat -ladm -lpicl
ifeq ($(SFW),YES)
SNMP_LIBS_CORE += -lpkcs11
endif
else
SNMP_LIBS_CORE = `net-snmp-config --agent-libs`
endif # CXX != CC
SNMP_LIBS = -lsnmp -lsnmp-smestattable -lsnmp-smeerrtable $(SNMP_LIBS_CORE)
CXXFLAGS := $(CXXFLAGS) -DSNMP
export SNMP_LIBS_CORE
export SNMP_LIBS
endif

ifeq ($(SMSX),YES)
CXXFLAGS := $(CXXFLAGS) -DSMSEXTRA
endif

ifeq ($(WARNINGS),YES)
ifeq ($(CXX),CC)
COMPFLAGS := $(COMPFLAGS) +w
else
COMPFLAGS := $(COMPFLAGS) -Wall
endif
endif

ifdef VERBOSE
export VERBOSE
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
CXXFLAGS := -g $(CXXFLAGS) -DDISABLE_ANY_CHECKS -fast
CFLAGS := -g $(CFLAGS) -DDISABLE_ANY_CHECKS -fast
else
CFLAGS := $(CFLAGS) -O2 -finline-limit=2048 -DDISABLE_ANY_CHECKS
CXXFLAGS := $(CXXFLAGS) -O2 -finline-limit=2048 -DDISABLE_ANY_CHECKS
endif
endif
endif
endif

ifeq ($(UNICODE),YES)
CFLAGS := $(CFLAGS) -DUNICODE
CXXFLAGS := $(CXXFLAGS) -DUNICODE
export UNICODE
endif

ifeq ($(MSAG_FAKE_STAT),YES)
CFLAGS := $(CFLAGS) -DMSAG_FAKE_STAT
CXXFLAGS := $(CXXFLAGS) -DMSAG_FAKE_STAT
export MSAG_FAKE_STAT
endif

ifeq ($(MSAG_INMAN_BILL),YES)
CFLAGS := $(CFLAGS) -DMSAG_INMAN_BILL
CXXFLAGS := $(CXXFLAGS) -DMSAG_INMAN_BILL
export MSAG_INMAN_BILL
MSAG_INMAN_LIBS := -linman-interaction-msgbill -linman-interaction -linman-common
endif

ifdef ASSERT
CFLAGS := $(CFLAGS) -DASSERT_THROW_IF_FAIL
CXXFLAGS := $(CFLAGS) -DASSERT_THROW_IF_FAIL
endif

#ifneq ($(NOMAP),YES)
#endif
PROC_TYPE := $(shell uname -p)
ifeq ($(PROC_TYPE),sparc)
CXXFLAGS := $(CXXFLAGS) -DSPARC
CFLAGS := $(CFLAGS) -DSPARC
endif

ifndef STLP_NEW_ALLOC
STLP_NEW_ALLOC=YES
endif

ifeq ($(STLP_NEW_ALLOC),YES)
CXXFLAGS := $(CXXFLAGS) -D_STLP_USE_NEWALLOC=1
endif

CXXFLAGS := $(CXXFLAGS) $(ORACLE_INC)
CFLAGS := $(CFLAGS) $(ORACLE_INC)

ifneq ("$(NOMAP)","YES")
MAP_LIBRARIES := -L$(SS7_STACK_HOME)/lib -l$(SS7_LIBRARY_NAME) -lsystem-mapio
CXXFLAGS := $(CXXFLAGS) -DUSE_MAP
endif

ifneq ($(CXX),CC)
CXXFLAGS := $(CXXFLAGS) -DTEMPINST
endif

ifdef EIN_HD
CXXFLAGS+=-DEIN_HD=$(EIN_HD)
CFLAGS+=-DEIN_HD=$(EIN_HD)
endif

export CXXFLAGS
export CFLAGS
export LDFLAGS
export COMPFLAGS
export MAP_LIBRARIES

#MODULES := $(shell cat modules-list | awk '/^[^\#]./{print $$1}' )

SMSC_BUILDDIR_LIST = $(foreach x,lib bin obj deps,$(SMSC_BUILDDIR)/$(x))

#$(warning $(MODULES))

#all:  build
#	@echo >> /dev/null

#__.empty.__:
#	@echo >> /dev/null

#commit:
#	@echo "commit cources into CVS"
#	cvs -ecat commit

#update:
#	@echo "updating sources from CVS"
#	cvs update -d

#docs:
#	@echo Generating docs
#	@mkdir -p $(SMSC_BUILDDIR)/doc/html
#	@perl -e 'while(<>){s!\$$\(DOCSDIR\)!$(SMSC_BUILDDIR)/doc/html!;print;}' <Doxyfile.tmpl >Doxyfile
#	@doxygen Doxyfile
#	@rm Doxyfile


#__build_message__:
#	@echo "build in SMSC"

#build: __build_message__ $(SMSC_BUILDDIR_LIST)
#	@if [ -n "$(MODULES)" ]; then \
#	    XXXX="$(MODULES)"; \
#	    set -e; for i in $$XXXX; do $(MAKE) -s $$i.rbuild ; done; \
#	fi

#ubuild: .update .build
#	@echo >> /dev/null

#clean:
#	@echo "clean all"
#	@rm -rf $(SMSC_BUILDDIR)/deps
#	@if [ -n "$(MODULES)" ]; then \
#	    XXXX="$(MODULES)"; \
#	    set -e; for i in $$XXXX; do $(MAKE) -s $$i.rclean ; done; \
#	fi

#deps:
#	@echo "build dependencies"
#	@if [ -n "$(MODULES)" ]; then \
#	    XXXX="$(MODULES)"; \
#	    set -e; for i in $$XXXX; do $(MAKE) -s $$i.deps ; done; \
#	fi


#$(MODULES) $(foreach x,$(MODULES),$(x).%): __.empty.__ $(SMSC_BUILDDIR_LIST)
#override _modules = $(subst ., ,$@)
#override _module = $(firstword $(_modules))
#override _subtarget = $(if $(word 2, $(_modules)), $(patsubst $(_module).%,%,$@), rbuild )
#	@if [ -d $(_module) ]; then \
#	    cd $(_module); $(MAKE) -s $(_subtarget); \
#	else \
#	    echo "'$(_module)' is not module"; exit 1;\
#	fi

all: build

webapp.newbuild:
	@cd ../webapps/smsc;perl $(SMSC_SRCDIR)/conf/newbuild.pl webapp


$(SMSC_BUILDDIR_LIST):
	@mkdir -p $(SMSC_BUILDDIR_LIST)

export USECOLORS

gen: $(SMSC_BUILDDIR_LIST)
	@echo Generating makefile.inc
	@perl conf/genmake.pl

deps:
	@echo make deps is deprecated. use make gen instead
	@echo Generating makefile.inc
	@perl conf/genmake.pl
        
ifneq ($(filter-out deps gen,$(MAKECMDGOALS)),)
-include makefile.inc
endif
