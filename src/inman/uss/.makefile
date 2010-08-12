#
# "@(#)$Id$"
#
# Automatically updates .hpp file containing build NUM and DATE
#
override PROD_PREFIX := USSMAN
override PROD_VER_HEADER := $(SMSC_SRCDIR)/inman/uss/ussversion.hpp
override BUILDID_HEADER := $(SMSC_SRCDIR)/inman/uss/ussBuildId.hpp

include $(SMSC_SRCDIR)/conf/gen_build_id.mak

