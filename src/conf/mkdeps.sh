#!/bin/bash

function mkdeps()
{
  if [ $ext == '.cpp' ] ; then
  $CXX $CXXFLAGS $COMPFLAGS $depflags $file.cpp 2>/dev/null | perl -e 'print (index($_,":")>-1 ? "$ENV{SMSC_BUILDDIR}/obj/$ENV{dir}/$_" : "$_") for(<>)' > $SMSC_BUILDDIR/deps/$file.depx 
  else
  $CC $CFLAGS $C_COMPFLAGS $depflags $file.c 2>/dev/null | perl -e 'print (index($_,":")>-1 ? "$ENV{SMSC_BUILDDIR}/obj/$ENV{dir}/$_" : "$_") for(<>)' > $SMSC_BUILDDIR/deps/$file.depx 
  fi
}

export dir=$1
export file=$2
export ext=$3
export depflags=-xM1
if [ "x$CXX" == "xgcc" -o "x$CXX" == "xg++" -o "x$CXX" == "xc++" ] ; then
   export depflags=-MM
fi
if mkdeps ; then
mv $SMSC_BUILDDIR/deps/$file.depx $SMSC_BUILDDIR/deps/$file.dep
else
rm $SMSC_BUILDDIR/deps/$file.depx
fi
