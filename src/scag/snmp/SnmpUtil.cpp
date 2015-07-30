#include "SnmpUtil.h"
#include "scag/config/base/ConfigManager2.h"

namespace scag2 {
namespace snmp {

static bool fillEmptyDataFlag = false;
static scag2::stat::StatisticsManager* statManPtr = 0;
const char* noStatData = "No-statistics-data";

scag2::stat::StatisticsManager* getStatMan()
{
  return statManPtr;
}
void setStatMan(scag2::stat::StatisticsManager* value)
{
  statManPtr = value;
}

bool fillEmptyData()
{
  return fillEmptyDataFlag;
}
void setEmptyDataFlag(bool value)
{
  fillEmptyDataFlag = value;
}

void uint64_to_U64(uint64_t val1, U64& val2)
{
  val2.high = (val1>>32)&0xffffffffUL;
  val2.low = val1&0xffffffffUL;
}

std::string oid2str(oid* oids, size_t len)
{
  std::string result = "";
  char buf[32];
  oid* ptr = oids;
  if ( 0 == len )
    result = "empty";
  else
  {
    snprintf(buf, 32, "%d", *ptr++);
    result = buf;
    for ( size_t i=1; i<len; ++i )
    {
      snprintf(buf, 32, ".%d", *ptr++);
      result += buf;
    }
  }
  return result;
}

std::string netsnmp_index2str(netsnmp_index oid_idx)
{
  return oid2str(oid_idx.oids, oid_idx.len);
}

void initConfigParams(int timeout, smsc::logger::Logger* log)
{
  int cacheTimeout = 60;
  try {
    cacheTimeout = config::ConfigManager::Instance().getConfig()->getInt("snmp.cacheTimeout");
  }
  catch (...) {
    smsc_log_info(log, "Config parameter snmp.cacheTimeout not found, using default=%d", cacheTimeout);
  }
  timeout = cacheTimeout; // seconds
  try {
    fillEmptyDataFlag = config::ConfigManager::Instance().getConfig()->getBool("snmp.fillEmptyData");
  }
  catch (...) {
    smsc_log_info(log, "Config parameter snmp.fillEmptyData not found, using default=%s", fillEmptyDataFlag?"true":"false");
  }
}

void logContainerError(container_info& info)
{
  smsc_log_error(info.log, "%s: CONTAINER_INSERT(%s) returns(%d)", info.name, info.sys_id, info.rc);
  std::string idxStr = (info.oidx_idx) ? netsnmp_index2str(*(info.oid_idx)) : "null";
  std::string oidStr = oid2str(info.oid_tmp, info.rec_index_len);
  smsc_log_error(info.log, "%s: indexes_set(%d) => (%d '%s' '%s')", info.name,
      info.index, info.rec_index, idxStr.c_str(), oidStr.c_str());
}

void logIndexDebug(smsc::logger::Logger* log, const char* msg, long index1, long index2, netsnmp_index oid_idx, oid* oid_tmp, size_t max_len)
{
  std::string idxStr = netsnmp_index2str(oid_idx);
  std::string oidStr = oid2str(oid_tmp, max_len);
  smsc_log_debug(log, "%s_indexes_set(%d) => (%d '%s' '%s')",
    msg, index1, index2, idxStr.c_str(), oidStr.c_str());
}

void combineCountersHash(counterHash& h0, counterHash& h1, counterHash& h2)
{
  char* key = 0;
  stat::CommonPerformanceCounter* counter1 = 0;
  stat::CommonPerformanceCounter* counter2 = 0;

  h1.First();
  while ( h1.Next(key, counter1) ) {
    h0.Insert(key, counter1);
  }
  h2.First();
  while ( h2.Next(key, counter2) ) {
    try {
      counter1 = h0.Get(key);
      counter1->add(counter2);
    }
    catch (...) {
    }
  }
}

}
}
