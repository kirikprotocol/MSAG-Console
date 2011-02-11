#
# "@(#)$Id$"
#
# Automatically updates .hpp file containing build NUM and DATE
#
# NOTE: following vars must be defined prior to inclusion of this makefile:
#
# override PREFIX := product name prefix, is added to name of build NUM and DATE defines
# override PROD_VER_HEADER := absolute name of file from CVS repository to extract BuildId tag
# override BUILDID_HEADER := absolute name of file from CVS repository to write build NUM and DATE defines
#

ifneq ($(filter-out %.clean %.rclean clean,$(MAKECMDGOALS)),)
  override GEN_BUILD_ID := $(shell $(SMSC_SRCDIR)/conf/gen_build_id.sh $(PROD_PREFIX) $(PROD_VER_HEADER) $(BUILDID_HEADER))
else
  override GEN_BUILD_ID := $(shell $(SMSC_SRCDIR)/conf/rm_build_id.sh $(BUILDID_HEADER))
endif
