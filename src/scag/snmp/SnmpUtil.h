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

class container_info {
public:
  smsc::logger::Logger* log;
  const char* name;
  const char* sys_id;
  long index;
  long rec_index;
  netsnmp_index* oid_idx;
  oid* oid_tmp;
  size_t rec_index_len;
  int rc;
  container_info(smsc::logger::Logger* log_, const char* name_,
      const char* sys_id_, long index_, long rec_index_,
      netsnmp_index oid_idx_, oid* oid_tmp_, size_t rec_index_len_, int rc_) :
   log(log_), name(name_), sys_id(sys_id_), index(index_), rec_index(rec_index_),
   oid_idx(oid_idx_), oid_tmp(oid_tmp_), rec_index_len(rec_index_len_), rc(rc_) {}
  ~container_info() {}
protected:
  container_info() {}
};

void logContainerError(container_info& info);
void logIndexDebug(smsc::logger::Logger* log, const char* msg, long index1, long index2, netsnmp_index oid_idx, oid* oid_tmp, size_t max_len);

scag2::stat::StatisticsManager* getStatMan();
void setStatMan(scag2::stat::StatisticsManager*);

typedef smsc::core::buffers::Hash<stat::CommonPerformanceCounter*> counterHash;
typedef smsc::core::buffers::IntHash<uint64_t> errorHash;

void combineCountersHash(counterHash& h0, counterHash& h1, counterHash& h2);

}
}

#endif /* !_SCAG_SNMP_UTIL_H */
