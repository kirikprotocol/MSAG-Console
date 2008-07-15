#!/bin/bash

function mkdeps()
{
  if [ $ext == '.cpp' ] ; then
  $CXX $CXXFLAGS $COMPFLAGS -xM1 $file.cpp | perl -e 'print "$ENV{SMSC_BUILDDIR}/obj/$ENV{dir}/$_"for(<>)' > $SMSC_BUILDDIR/deps/$file.depx
  else
  $CC $CFLAGS $C_COMPFLAGS -xM1 $file.c | perl -e 'print "$ENV{SMSC_BUILDDIR}/obj/$ENV{dir}/$_"for(<>)' > $SMSC_BUILDDIR/deps/$file.depx
  fi
}

export dir=$1
export file=$2
export ext=$3
if mkdeps ; then
mv $SMSC_BUILDDIR/deps/$file.depx $SMSC_BUILDDIR/deps/$file.dep
else
rm $SMSC_BUILDDIR/deps/$file.depx
fi
