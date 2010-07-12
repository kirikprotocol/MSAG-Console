#!/bin/bash

function usage()
{
   echo "Usage: $0 STOREDIR ENTRIESPERDIR"
}


if [ ! -d "$1" ]; then
   echo "Directory $1 does not exist" >&2
   usage
   exit -1
fi

if ( ! expr "$2" + 0 >/dev/null 2>&1 ) ; then
   echo "Second argument is not a number" >&2
   usage
   exit -1
fi

if [ $2 -le 0 ]; then
   echo "Second argument must be positive" >&2
   usage
   exit -1
fi

# --- arguments are processed
for d in `ls -1 "$1" | egrep '[^/][0-9][0-9]*$'` ; do
   test -d "$1/$d" || continue
   base=`expr $d / $2`
   bdir=`printf "check%010u" $base`
   if [ ! -d "$1/$bdir" ] ; then
      echo mkdir "$1/$bdir"
   fi
   echo mv "$1/$d" "$1/$bdir/"
done
