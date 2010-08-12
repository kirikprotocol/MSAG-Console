#
# "@(#)$Id$"
#
# Automatically updates .hpp file containing build NUM and DATE
#
override PROD_PREFIX := SMSC_2G
override PROD_VER_HEADER := $(SMSC_SRCDIR)/smsc/version.h
override BUILDID_HEADER := $(SMSC_SRCDIR)/smsc/smsc2gBuildId.hpp

include $(SMSC_SRCDIR)/conf/gen_build_id.mak

