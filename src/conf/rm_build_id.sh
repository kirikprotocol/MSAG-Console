#!/bin/sh
#
# "@(#)$Id$"
#
# SCRIPT: erases a .hpp file with build ids defines
#
# USAGE: rm_build_id.sh  buildId_file_absname
#
#   buildId_file_absname - absolute name of .hpp file with preprocessor defines, based on SMSC_SRCDIR
#
# VARIABLES:  SMSC_SRCDIR, SMSC_BUILDDIR
#
if [ -z "$1" ]
then
  echo "$0: BuildId file name isn't specified"
  exit 1
fi

#determine name of buildid_file relative to $SMSC_SRCDIR
BUILDID_FILE=`echo $SMSC_SRCDIR $1 | awk '{ pos=index($2,$1); if (pos == 1) { pos += length($1); fn = substr($2, pos); pos = index(fn, "/"); if (pos == 1) print substr(fn, 2); else print fn; }}'`

if [ -z "$BUILDID_FILE" ]
then
  echo "$0: BuildId file name is invalid"
  exit 1
fi
echo Cleaning $BUILDID_FILE >&2

if [ -f $SMSC_BUILDDIR/deps/$BUILDID_FILE ]
then # Erase BuildId file
  rm -f $SMSC_BUILDDIR/deps/$BUILDID_FILE
fi
exit 0

