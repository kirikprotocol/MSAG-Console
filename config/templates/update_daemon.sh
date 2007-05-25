#!/bin/bash

mkdir bin/old

mv bin/smsc_ssdaemon.cur     bin/old
mv bin/smsc_ssdaemon         bin/smsc_ssdaemon.cur

mv bin/hs_smsc_ssdaemon.cur  bin/old
mv bin/hs_smsc_ssdaemon      bin/hs_smsc_ssdaemon.cur

mv bin/hsadmin.cur	     bin/old
mv bin/hsadmin		     bin/hsadmin.cur

mv    lib/liblogger.so       bin/old
cp ../lib/liblogger.so       lib/liblogger.so