#ifndef __SMSC_SNMP_SMSCSNMP_SNMPTRAP_H__
#define __SMSC_SNMP_SMSCSNMP_SNMPTRAP_H__

extern "C" void SnmpTrap(const char* prefix,const char* obj,const char* alarmId,int svrt,const char* fmt,...);

#endif
