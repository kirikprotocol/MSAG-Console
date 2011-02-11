#!/bin/sh
#
# "@(#)$Id$"
#
# SCRIPT: extracts from specified file stored in CVS repository last build ID tag
#         and creates a .hpp file with two C preprocessor defines
#   #define $(PRODUCT_PREFIX)_BUILD_NUM
#   #define $(PRODUCT_PREFIX)_BUILD_DATE
#
# USAGE: gen_build_id.sh  PRODUCT_PREFIX  version_file_absname  buildId_file_absname
#
#   version_file_absname - absolute name of file with CVS tag, based on SMSC_SRCDIR
#   buildId_file_absname - absolute name of .hpp file with preprocessor defines, based on SMSC_SRCDIR
#
# VARIABLES:  SMSC_SRCDIR, SMSC_BUILDDIR, SMSC_BUILD_TAG_FORMAT
#

#echo SRCDIR: $SMSC_SRCDIR >&2
#echo ARGS: $1 $2 $3 >&2
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

#determine name of version_file relative to $SMSC_SRCDIR
VERSION_FILE=`echo $SMSC_SRCDIR $2 | awk '{ pos=index($2,$1); if (pos == 1) { pos += length($1); fn = substr($2, pos); pos = index(fn, "/"); if (pos == 1) print substr(fn, 2); else print fn; }}'`
#echo $VERSION_FILE >&2

if [ -z "$VERSION_FILE" ]
then
  echo "$0: Version file name is invalid"
  exit 4
fi
if [ ! -f "$VERSION_FILE" ]
then
  echo "$0: Version file is not found"
  exit 4
fi

#determine name of buildid_file relative to $SMSC_SRCDIR
BUILDID_FILE=`echo $SMSC_SRCDIR $3 | awk '{ pos=index($2,$1); if (pos == 1) { pos += length($1); fn = substr($2, pos); pos = index(fn, "/"); if (pos == 1) print substr(fn, 2); else print fn; }}'`

if [ -z "$BUILDID_FILE" ]
then
  echo "$0: BuildId file name is invalid"
  exit 5
fi
#echo $VERSION_FILE $BUILDID_FILE >&2

if [ ! -f $3 ] || [ ! -f $SMSC_BUILDDIR/deps/$BUILDID_FILE ]
then # Regenerate BuildId file
  echo Generating $VERSION_FILE $BUILDID_FILE >&2
  BUILD_DATE=`LANG=C; date '+%b %e %Y'`
  BUILD_NUM=`cvs status -v $VERSION_FILE | egrep $SMSC_BUILD_TAG_FORMAT | awk 'BEGIN{nv=0;} { if (!nv) {for (i = 1; i < NF; ++i) print $i; nv++;}}' | egrep $SMSC_BUILD_TAG_FORMAT`
  if [ -z "$BUILD_NUM" ]
  then
    echo "$0: $VERSION_FILE doesn't have valid buildId CVS tag assigned"
    exit 6
  fi

  BUILD_PATH=`echo $SMSC_BUILDDIR/deps/$BUILDID_FILE | awk 'BEGIN{FS="/"}{ printf("%s", $1); for(i = 2; i < NF; ++i) printf("/%s", $i);}'`
#  echo BUILD_PATH: $BUILD_PATH  >&2
  mkdir -p $BUILD_PATH
  echo "#define $1_BUILD_NUM   $BUILD_NUM" > $SMSC_BUILDDIR/deps/$BUILDID_FILE
  echo "#define $1_BUILD_DATE  \"$BUILD_DATE\"" >> $SMSC_BUILDDIR/deps/$BUILDID_FILE
  cp $SMSC_BUILDDIR/deps/$BUILDID_FILE $3
else
  echo Using $VERSION_FILE $BUILDID_FILE >&2
fi

cat $3 >&2
exit 0

