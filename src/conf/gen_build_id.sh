#!/bin/sh
#
# "@(#)$Id$"
#
# SCRIPT: extracts from specified file from CVS repository last build ID tag
#         and creates a .hpp file with two C preprocessor defines
#   #define $(PRODUCT_PREFIX)_BUILD_NUM
#   #define $(PRODUCT_PREFIX)_BUILD_DATE
#
# USAGE: gen_build_id.sh  PRODUCT_PREFIX  version_file_absname  buildId_file_absname
#
# VARIABLES:  SMSC_SRCDIR, SMSC_BUILD_TAG_FORMAT
#
if [ -z "$1" ]
then
  echo "$0: Product prefix isn't specified"
  exit 1
fi

if [ -z "$2" ]
then
  echo "$0: Version file name isn't specified"
  exit 2
fi

if [ -z "$3" ]
then
  echo "$0: BuildId file name isn't specified"
  exit 3
fi

if [ -z "$SMSC_BUILD_TAG_FORMAT" ]
then
  SMSC_BUILD_TAG_FORMAT='B[0-9]*'
fi

VERSION_FILE=`echo $SMSC_SRCDIR $2 | awk '{ pos=index($2,$1); if (pos == 1) { pos += length($1); fn = substr($2, pos); pos = index(fn, "/"); if (pos == 1) print substr(fn, 2); else print fn; }}'`
#echo $VERSION_FILE

if [ -z "$VERSION_FILE" ]
then
  echo "$0: Version file name is invalid"
  exit 4
fi

BUILD_DATE=`date '+%b %e %Y'`
BUILD_NUM=`cvs status -v $VERSION_FILE | egrep $SMSC_BUILD_TAG_FORMAT | awk 'BEGIN{nv=0;} { if (!nv) {print $1; nv++;}}'`
if [ -z "$BUILD_NUM" ]
then
  echo "$0: $VERSION_FILE doesn't have valid buildId CVS tag assigned"
  exit 5
fi
echo $1 Build properties: $BUILD_NUM $BUILD_DATE >&2

echo "#define $1_BUILD_NUM   $BUILD_NUM" > $3
echo "#define $1_BUILD_DATE  \"$BUILD_DATE\"" >> $3

