#
# "@(#)$Id$"
#
# Automatically updates .hpp file containing build NUM and DATE
#
override PROD_PREFIX := INMAN
override PROD_VER_HEADER := $(SMSC_SRCDIR)/inman/version.hpp
override BUILDID_HEADER := $(SMSC_SRCDIR)/inman/inmBuildId.hpp

include $(SMSC_SRCDIR)/conf/gen_build_id.mak

