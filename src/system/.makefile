#
# "@(#)$Id$"
#
# Automatically updates .hpp file containing build NUM and DATE
#
override PROD_PREFIX := SMSC
override PROD_VER_HEADER := $(SMSC_SRCDIR)/system/version.h
override BUILDID_HEADER := $(SMSC_SRCDIR)/system/smscBuildId.hpp

include $(SMSC_SRCDIR)/conf/gen_build_id.mak

