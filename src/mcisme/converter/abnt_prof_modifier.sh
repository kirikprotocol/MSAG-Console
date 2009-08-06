#!/bin/sh

if [ $# -lt 4 ] ; then
  echo "Usage: $0 abnts_lst_file dir_to_abntprof_db abnt_prof_db_filename on/off" 
  exit 1
fi

dos2unix -ascii $1 $1.unx 2>/dev/null
if [ $? -ne 0 ] ; then
  echo "can't convert file to unix format"
  exit 1
fi

if [ $4 != "on" ] && [ $4 != "off" ] ; then
  echo "last argument must have value off or on"
fi

dump_MCIProfdb $2 $3 $4 $1.unx

rm -f $1.unx
