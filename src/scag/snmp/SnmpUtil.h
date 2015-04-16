#ifndef _SCAG_SNMP_UTIL_H
#define _SCAG_SNMP_UTIL_H

#include <stdio.h>
#include <string>
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/types.h"
#include "net-snmp/library/int64.h"
#include "logger/Logger.h"

namespace scag2 {
namespace snmp {

static const char* noStatData = "No-statistics-data";
static bool fillEmptyData = false;
//static const char* noStatData;
//static bool fillEmptyData;

void uint64_to_U64(uint64_t val1, U64& val2);
std::string oid2str(oid* oids, size_t len);
std::string netsnmp_index2str(netsnmp_index oid_idx);
void initConfigParams(int timeout, smsc::logger::Logger* log);

void logIndexDebug(smsc::logger::Logger* log, const char* msg, long index1, long index2, netsnmp_index oid_idx, oid* oid_tmp, size_t max_len);

}
}

#endif /* !_SCAG_SNMP_UTIL_H */
