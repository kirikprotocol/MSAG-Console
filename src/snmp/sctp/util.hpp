#ifndef __SMSC_SNMP_SCTP_UTIL_HPP__
#define __SMSC_SNMP_SCTP_UTIL_HPP__
#ident "$Id$"
#include <string>
#include "ss7cp.h"
#include "mgmtapi.h"
namespace smsc{
namespace snmp{
namespace sctp{
extern const char* getModuleName(USHORT_T moduleId);
extern const char* getReturnCodeDescription(USHORT_T code);
extern const char* getResultDescription(UCHAR_T result);
extern const char * getStackStatusDescription(UCHAR_T mmState);
extern const char * getAssociationStateDescription(UCHAR_T state);
extern const char * getTypeOfServiceDescription(UCHAR_T typeOfService);
extern std::string getAlarmResultDescription(UCHAR_T noOfAlarms, ALARMS_T *alarms_sp);
extern std::string dump(USHORT_T len, UCHAR_T* udp);
}//namespace sctp
}//namespace snmp
}//namespace smsc
#endif /* __SMSC_SNMP_SCTP_UTIL_HPP__ */
