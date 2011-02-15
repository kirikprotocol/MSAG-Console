#!/bin/sh
#
# "@(#)$Id$"
#
# SCRIPT: extracts from specified file stored in CVS repository last build ID tag
#         and creates a .hpp file with two C preprocessor defines
#   #define $(PRODUCT_PREFIX)_BUILD_NUM
#   #define $(PRODUCT_PREFIX)_BUILD_DATE
#
# USAGE: gen_build_id.sh  package_name PRODUCT_PREFIX version_file_shortname  buildId_file_shortname
#
#   package_name - relative to SMSC_SRCDIR name of package(CVS module)
#   version_file_shortname - relative to SMSC_SRCDIR name of .cpp file with CVS tag
#   buildId_file_shortname - relative to SMSC_SRCDIR name of .hpp file with preprocessor defines
#
# VARIABLES:  SMSC_SRCDIR, SMSC_BUILDDIR, SMSC_BUILD_TAG_FORMAT
#

#echo SRCDIR: $SMSC_SRCDIR >&2
#echo ARGS: $1 $2 $3 $4 >&2
if [ -z "$1" ]
then
  echo "$0: Package name isn't specified"
  exit 1
fi
PACKAGE_NAME=$1

if [ -z "$2" ]
then
  echo "$0: Product prefix isn't specified"
  exit 1
fi

if [ -z "$3" ]
then
  echo "$0: Version file name isn't specified"
  exit 1
fi
VERSION_FILE=$3

if [ -z "$4" ]
then
  echo "$0: BuildId file name isn't specified"
  exit 1
fi
BUILDID_FILE=$4

if [ -z "$SMSC_BUILD_TAG_FORMAT" ]
then
  SMSC_BUILD_TAG_FORMAT='B[0-9]*'
fi

if [ ! -f $SMSC_SRCDIR/$PACKAGE_NAME/$VERSION_FILE ]
then
  echo "$0: Version file is not found: $SMSC_SRCDIR/$PACKAGE_NAME/$VERSION_FILE"
  exit 2
fi

BUILD_PATH=$SMSC_BUILDDIR/deps/$PACKAGE_NAME

if [ ! -f $SMSC_SRCDIR/$PACKAGE_NAME/$BUILDID_FILE ] || [ ! -f $BUILD_PATH/$BUILDID_FILE ]
then # Regenerate BuildId file
  echo Generating $BUILDID_FILE from $VERSION_FILE  >&2
  BUILD_DATE=`LANG=C; date '+%b %e %Y'`
  BUILD_NUM=`cvs status -v $PACKAGE_NAME/$VERSION_FILE | egrep $SMSC_BUILD_TAG_FORMAT | awk 'BEGIN{nv=0;} { if (!nv) {for (i = 1; i < NF; ++i) print $i; nv++;}}' | egrep $SMSC_BUILD_TAG_FORMAT`
  if [ -z "$BUILD_NUM" ]
  then
    echo "$0: $PACKAGE_NAME/$VERSION_FILE doesn't have valid buildId CVS tag assigned"
    exit 6
  fi
  
  mkdir -p $BUILD_PATH
  echo "#define $2_BUILD_NUM   $BUILD_NUM" > $BUILD_PATH/$BUILDID_FILE
  echo "#define $2_BUILD_DATE  \"$BUILD_DATE\"" >> $BUILD_PATH/$BUILDID_FILE
  cp $BUILD_PATH/$BUILDID_FILE $SMSC_SRCDIR/$PACKAGE_NAME/$BUILDID_FILE
else
  echo Using $BUILDID_FILE >&2
fi

cat $SMSC_SRCDIR/$PACKAGE_NAME/$BUILDID_FILE >&2
exit 0

