#!/bin/bash
if [ -f /data/store/store.bin.bak ]; then
  DATE=`perl -e '($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);printf("%04d%02d%02d%02d%02d%02d",$year+1900,$mon+1,$mday,$hour,$min,$sec)'`
  mv /data/store/store.bin.bak /data/store/store.bin_bak_$DATE
  if [ -f /data/store/store.bin ]; then
    mv /data/store/store.bin /data/store/store.bin_bad_$DATE
    bin/RepairAndMergeStore /data/store/store.bin /data/store/store.bin_bak_$DATE /data/store/store.bin_bad_$DATE
  else 
    bin/RepairAndMergeStore /data/store/store.bin /data/store/store.bin_bak_$DATE
  fi
fi
