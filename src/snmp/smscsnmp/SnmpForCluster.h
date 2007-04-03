#ifndef __SMSC_SNMP_SMSCSNMP_SNMPFORCLUSTER_H__
#define __SMSC_SNMP_SMSCSNMP_SNMPFORCLUSTER_H__
#include "SnmpTrap.h"
#include <rgm/libdsdev.h>

#define SNMP_REPORT_SERVICE_START \
  SnmpTrap(scds_get_resource_type_name(scds_handle),"SYSTEM",\
  scds_get_resource_type_name(scds_handle),1,\
  "CLEARED SYSTEM %s Started (AlarmId=%s; severity=1)",\
  scds_get_resource_type_name(scds_handle),scds_get_resource_type_name(scds_handle))

#define SNMP_REPORT_SERVICE_STOP \
  SnmpTrap(scds_get_resource_type_name(scds_handle),"SYSTEM",\
  scds_get_resource_type_name(scds_handle),5,\
  "ACTIVE SYSTEM %s Stopped (AlarmId=%s; severity=5)",\
  scds_get_resource_type_name(scds_handle),scds_get_resource_type_name(scds_handle))

#define SNMP_REPORT_SERVICE_FAILURE \
  if(rc==SCDS_PROBE_COMPLETE_FAILURE) \
  {\
    SnmpTrap(scds_get_resource_type_name(scds_handle),"SYSTEM",\
    scds_get_resource_type_name(scds_handle),5,\
    "ACTIVE SYSTEM %s Failure (AlarmId=%s; severity=5)",\
    scds_get_resource_type_name(scds_handle),scds_get_resource_type_name(scds_handle)); \
  }else \
  { \
  SnmpTrap(scds_get_resource_type_name(scds_handle),"SYSTEM",\
  scds_get_resource_type_name(scds_handle),5,\
  "ACTIVE SYSTEM %s Partial Failure (AlarmId=%s; severity=2)",\
  scds_get_resource_type_name(scds_handle),scds_get_resource_type_name(scds_handle)); \
  }

#endif
