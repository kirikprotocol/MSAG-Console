
#
# $Id$
#
#

PACKAGE := emailsme

__LIBRARIES :=-L$(SMSC_BUILDDIR)/bin -llogger -L$(SMSC_BUILDDIR)/bin/db/exceptions/ -ldb_exc

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
