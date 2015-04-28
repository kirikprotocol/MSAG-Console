#ifndef _SCAG_SNMP_UTIL_H
#define _SCAG_SNMP_UTIL_H

#include <stdio.h>
#include <string>
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/types.h"
#include "net-snmp/library/int64.h"
#include "logger/Logger.h"
#include "scag/stat/impl/StatisticsManager.h"
#include "scag/stat/impl/Performance.h"

namespace scag2 {
namespace snmp {

extern const char* noStatData;

bool fillEmptyData();
void setEmptyDataFlag(bool value);

void uint64_to_U64(uint64_t val1, U64& val2);
std::string oid2str(oid* oids, size_t len);
std::string netsnmp_index2str(netsnmp_index oid_idx);
void initConfigParams(int timeout, smsc::logger::Logger* log);

void logIndexDebug(smsc::logger::Logger* log, const char* msg, long index1, long index2, netsnmp_index oid_idx, oid* oid_tmp, size_t max_len);

scag2::stat::StatisticsManager* getStatMan();
void setStatMan(scag2::stat::StatisticsManager*);

typedef smsc::core::buffers::Hash<stat::CommonPerformanceCounter*> counterHash;
typedef smsc::core::buffers::IntHash<uint64_t> errorHash;

void combineCountersHash(counterHash& h0, counterHash& h1, counterHash& h2);

}
}

#endif /* !_SCAG_SNMP_UTIL_H */
