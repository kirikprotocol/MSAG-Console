#
# $Id$
#
CXXFLAGS := -I. -I../../asn1rt -I../../codec_inc/ros -I../../codec_inc/in_cs -I../../codec_inc/map -I../../codec_inc/cap  $(CXXFLAGS)
CFLAGS := -I. -I../../asn1rt -I../../codec_inc/ros -I../../codec_inc/in_cs -I../../codec_inc/map -I../../codec_inc/cap  $(CFLAGS)
PACKAGE := inman.comp.cap_sms

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
