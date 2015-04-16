#include "SnmpUtil.h"
#include "scag/config/base/ConfigManager2.h"

namespace scag2 {
namespace snmp {

//const char* noStatData = "No-statistics-data";
//bool fillEmptyData = false;

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
    result += buf;
  }
  for ( size_t i=1; i<len; ++i )
  {
    snprintf(buf, 32, ".%d", *ptr++);
    result += buf;
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
    fillEmptyData = config::ConfigManager::Instance().getConfig()->getBool("snmp.fillEmptyData");
  }
  catch (...) {
    smsc_log_info(log, "Config parameter snmp.fillEmptyData not found, using default=%s", fillEmptyData?"true":"false");
  }
}

void logIndexDebug(smsc::logger::Logger* log, const char* msg, long index1, long index2, netsnmp_index oid_idx, oid* oid_tmp, size_t max_len)
{
  std::string idxStr = netsnmp_index2str(oid_idx);
  std::string oidStr = oid2str(oid_tmp, max_len);
  smsc_log_debug(log, "%s_indexes_set(%d) => (%d '%s' '%s')",
    msg, index1, index2, idxStr.c_str(), oidStr.c_str());
}


}
}
