This directory contains SNMP management module
1. Download and install net-snmp package
2. Stop Sun's snmp agent
3. Set up OpenSSL library path (optional)
4. Set up MIB and snmpd config
5. Register and start net-snmp agent



------------------------------------------------------------
1. Download and install net-snmp package

su -
cd /
wget http://belnet.dl.sourceforge.net/sourceforge/net-snmp/net-snmp_5.2.2-SunOS_5.9_sun4u.tar.gz
gzip -d net-snmp_5.2.2-SunOS_5.9_sun4u.tar.gz
tar -xvf net-snmp_5.2.2-SunOS_5.9_sun4u.tar

------------------------------------------------------------
2. Stop Sun's snmp agent

Solaris 9

cd /etc/rc3.d
./S76snmpdx stop
./S77dmi stop
mv S76snmpdx s76snmpdx
mv S77dmi s77dmi

Solaris 10

svcadm disable sma
svcadm disable dmi
svcadm disable snmpdx

------------------------------------------------------------
3. Set up OpenSSL library path (optional)

If snmpd is linked with specific version of OpenSSL just link shared lib to /usr/lib, for example

cd /usr/lib
ln -s /usr/local/ssl/lib/libcrypto.so.0.9.7 libcrypto.so.0.9.8

------------------------------------------------------------
4. Set up MIB and snmpd config

Obtain IP address of host to which send SNMP traps and
register them in /etc/hosts with traphost name, for example

192.168.33.17   traphost        # SNMP trap sink

Copy snmpd.conf to /usr/local/share/snmp/snmpd.conf
Copy SIBINCO-SMSC-MIB.txt to /usr/local/share/snmp/mibs/SIBINCO-SMSC-MIB.txt
NOTE: Solaris 10 has built-in net-snmp and directory to put cfg,mib can be /etc/sma/snmp

Check OID tree parsing
snmptranslate -Tp SIBINCO-SMSC-MIB::sibinco
snmptranslate -m SIBINCO-SMSC-MIB -Tp -IR sibinco

------------------------------------------------------------
5. Register and start net-snmp agent

cd /etc/rc3.d
cat > S78net-snmp
#!/sbin/sh
#
# /etc/rc3.d/S78net-snmp
#
# pkill has moved over the years so we need to find it
#
X=`which pkill`

case "$1" in
'start')
        /usr/local/sbin/snmpd
        ;;

'stop')
        $X -TERM -x -u 0 snmpd
        ;;

*)
        echo "Usage: $0 { start | stop }"
        exit 1
        ;;
esac
exit 0
^D
chmod 744 S78net-snmp
./S78net-snmp start

Get var values
snmpwalk -v 2c -c ussdc localhost SIBINCO-SMSC-MIB::sibinco
 
############################################################
                        Appendix
############################################################

Q: How to run net-snmp master agent
A: /usr/local/sbin/snmpd -d -D -V -Dagentx -f -L
