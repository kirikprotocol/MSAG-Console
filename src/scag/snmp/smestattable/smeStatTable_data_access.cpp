/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 14170 $ of $ 
 *
 * $Id$
 */
/* standard Net-SNMP includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* include our parent header */
#include "smeStatTable.hpp"
#include "smeStatTable_data_access.hpp"
#include "smeStatTable_subagent.hpp"
#include "util/int.h"

#include "scag/stat/impl/StatisticsManager.h"
#include "scag/stat/impl/StatCountersEnum.hpp"
#include "logger/Logger.h"
#include "core/buffers/Hash.hpp"

#include "scag/stat/impl/Performance.h"

#include "scag/config/base/ConfigManager2.h"


/** @ingroup interface
 * @addtogroup data_access data_access: Routines to access data
 *
 * These routines are used to locate the data used to satisfy
 * requests.
 * 
 * @{
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table smeStatTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * EYELINE-MSAG-MIB::smeStatTable is subid 10 of msag.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.2.10, length: 9
*/

namespace scag2 {
namespace snmp {
namespace smestattable {

smsc::logger::Logger* log;

void uint64_to_U64(uint64_t val1, U64& val2)
{
  val2.high = (val1>>32)&0xffffffffUL;
  val2.low = val1&0xffffffffUL;
}

void fill64(U64& counter, uint16_t value)
{
  U64 tmp64 = {0,0};
  uint64_t tmp = value;
  uint64_to_U64(tmp, tmp64);
  counter.high = tmp64.high;
  counter.low =  tmp64.low;
  smsc_log_debug(log, "fill64(%d) => %lld => (%u,%u) => (%u,%u)",
      value, tmp, tmp64.high, tmp64.low, counter.high, counter.low);
}

bool fillRecord(smeStatTable_rowreq_ctx* rec, char* sysId, stat::CommonPerformanceCounter* counter)
{
  smsc_log_debug(log, "fillRecord() sysId %s counter %p count %d(%d)",
      sysId?sysId:"empty", counter, counter?counter->count:0, stat::Counters::cntSmppSize);
  if (!sysId)
    return false;
  if (!counter)
    return false;
  if (counter->count < stat::Counters::cntSmppSize)
    return false;

  char smeStatSystemId[34];
  size_t smeStatSystemId_len = (strlen(sysId)<32) ? strlen(sysId) : 32;
  strncpy(&smeStatSystemId[0], sysId, smeStatSystemId_len);

/*
  if (
       (NULL == rec->data.smeStatSystemId)
       || (rec->data.smeStatSystemId_len < (smeStatSystemId_len * sizeof(smeStatSystemId[0])))
     )
  {
    snmp_log(LOG_ERR,"not enough space for value\n");
    return false;
  }
*/

  rec->data.smeStatSystemId_len = smeStatSystemId_len * sizeof(smeStatSystemId[0]);
  memcpy( rec->data.smeStatSystemId, smeStatSystemId, smeStatSystemId_len * sizeof(smeStatSystemId[0]) );

  fill64(rec->data.smeStatAccepted, counter->counters[stat::Counters::cntAccepted]);
  fill64(rec->data.smeStatRejected, counter->counters[stat::Counters::cntRejected]);
  fill64(rec->data.smeStatDelivered, counter->counters[stat::Counters::cntDelivered]);
  fill64(rec->data.smeStatGwRejected, counter->counters[stat::Counters::cntGw_Rejected]);
  fill64(rec->data.smeStatFailed, counter->counters[stat::Counters::cntFailed]);
  fill64(rec->data.smeStatRecieptOk, counter->counters[stat::Counters::cntRecieptOk]);
  fill64(rec->data.smeStatRecieptFailed, counter->counters[stat::Counters::cntRecieptFailed]);

/*
  for (uint32_t i = 0; i < counter->count; i++)
  {
    buf.WriteNetInt16((counter) ? counter->counters[i]:0);
    TimeSlotCounter<int>* cnt = (counter && counter->slots[i]) ? counter->slots[i] : 0;
    buf.WriteNetInt16((cnt) ? (uint16_t)cnt->Avg():0);
  }
*/
  smsc_log_debug(log, "fillRecord() OK");
  return true;
}


/**
 * initialization for smeStatTable data access
 *
 * This function is called during startup to allow you to
 * allocate any resources you need for the data table.
 *
 * @param smeStatTable_reg
 *        Pointer to smeStatTable_registration
 *
 * @retval MFD_SUCCESS : success.
 * @retval MFD_ERROR   : unrecoverable error.
 */
int smeStatTable_init_data(smeStatTable_registration_ptr smeStatTable_reg)
{
    DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_init_data","called\n"));

    /*
     * TODO:303:o: Initialize smeStatTable data.
     */
    /*
    ***************************************************
    ***             START EXAMPLE CODE              ***
    ***---------------------------------------------***/
    /*
     * if you are the sole writer for the file, you could
     * open it here. However, as stated earlier, we are assuming
     * the worst case, which in this case means that the file is
     * written to by someone else, and might not even exist when
     * we start up. So we can't do anything here.
     */
    /*
    ***---------------------------------------------***
    ***              END  EXAMPLE CODE              ***
    ***************************************************/

    log = smsc::logger::Logger::getInstance("snmp.stbl");
/*
    try
    {
      scag2::stat::StatisticsManager* statMan = SmeStatTableSubagent::getStatMan();
    }
    catch(...)
    {
      smsc_log_error(log, "smeStatTable_cache_load: getStatisticsManager exception");
      snmp_log(LOG_ERR, "smeStatTable_cache_load: Statistics Manager exception\n");
      return MFD_RESOURCE_UNAVAILABLE;
    }
    smsc_log_debug(log, "smeStatTable_cache_load: Statistics Manager OK");
*/
    return MFD_SUCCESS;
} /* smeStatTable_init_data */

/**
 * container overview
 *
 */

/**
 * container initialization
 *
 * @param container_ptr_ptr A pointer to a container pointer. If you
 *        create a custom container, use this parameter to return it
 *        to the MFD helper. If set to NULL, the MFD helper will
 *        allocate a container for you.
 * @param  cache A pointer to a cache structure. You can set the timeout
 *         and other cache flags using this pointer.
 *
 *  This function is called at startup to allow you to customize certain
 *  aspects of the access method. For the most part, it is for advanced
 *  users. The default code should suffice for most cases. If no custom
 *  container is allocated, the MFD code will create one for your.
 *
 *  This is also the place to set up cache behavior. The default, to
 *  simply set the cache timeout, will work well with the default
 *  container. If you are using a custom container, you may want to
 *  look at the cache helper documentation to see if there are any
 *  flags you want to set.
 *
 * @remark
 *  This would also be a good place to do any initialization needed
 *  for you data source. For example, opening a connection to another
 *  process that will supply the data, opening a database, etc.
 */
void smeStatTable_container_init(netsnmp_container **container_ptr_ptr, netsnmp_cache *cache)
{
    DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_container_init","called\n"));
    
    if((NULL == cache) || (NULL == container_ptr_ptr)) {
//    if (NULL == container_ptr_ptr) {
      snmp_log(LOG_ERR, "bad container param to smeStatTable_container_init\n");
      return;
    }

    /*
     * For advanced users, you can use a custom container. If you
     * do not create one, one will be created for you.
     */
    *container_ptr_ptr = NULL;

    /*
     * TODO:345:A: Set up smeStatTable cache properties.
     *
     * Also for advanced users, you can set parameters for the
     * cache. Do not change the magic pointer, as it is used
     * by the MFD helper. To completely disable caching, set
     * cache->enabled to 0.
     */
//    cache->timeout = SMESTATTABLE_CACHE_TIMEOUT; /* seconds */
//    config::ConfigManager& cfg = config::ConfigManager::Instance();
    int cacheTimeout = 60;
    try {
      cacheTimeout = config::ConfigManager::Instance().getConfig()->getInt("snmp.cacheTimeout");
    }
    catch (...) {
      __warning2__("Config parameter snmp.cacheTimeout not found, using default=%d", cacheTimeout);
      snmp_log(LOG_ERR, "Config parameter snmp.cacheTimeout not found, using default=%d\n", cacheTimeout);
    }
    cache->timeout = cacheTimeout; /* seconds */
} /* smeStatTable_container_init */

void smeStatTable_container_shutdown(netsnmp_container *container_ptr)
{
  DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_container_shutdown","called\n"));
  if (NULL == container_ptr) {
    snmp_log(LOG_ERR,"bad params to smeStatTable_container_shutdown\n");
    return;
  }
} /* smeStatTable_container_shutdown */

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

void fakeFillHashIfEmpty(smsc::core::buffers::Hash<stat::CommonPerformanceCounter*>& h)
{
  if ( h.GetCount() > 0 ) return;
  stat::CommonPerformanceCounter* counter = 0;
  smsc_log_debug(log, "smeStatTable_cache_load: no records, make 1 fake counter");
  counter = new stat::CommonPerformanceCounter(7);
  for ( int i=0; i<7; ++i ) counter->counters[i] = i+1;
  h.Insert("fakeRecord1", counter);
  counter = new stat::CommonPerformanceCounter(7);
  for ( int i=0; i<7; ++i ) counter->counters[i] = i+10;
  h.Insert("fakeRecord2", counter);
  counter = new stat::CommonPerformanceCounter(7);
  for ( int i=0; i<7; ++i ) counter->counters[i] = i+100;
  h.Insert("fakeRecord3", counter);
}


int smeStatTable_cache_load(netsnmp_container* container)
{
  if (container)
  {
    if ( !container->container_name )
      container->container_name = "smeStatTableContainer";
    smsc_log_debug(log, "smeStatTable_cache_load container %s", container->container_name);
  }
  else
  {
    smsc_log_error(log, "smeStatTable_cache_load error: container is NULL");
    return MFD_RESOURCE_UNAVAILABLE;
  }
  smsc::core::buffers::Hash<stat::CommonPerformanceCounter*>& h = SmeStatTableSubagent::getStatMan()->getCounters(0);
  smsc_log_debug(log, "smeStatTable_cache_load: getCounters(0) ok");

  long   smeStatIndex = 0;
  size_t recCount = 0;
  smeStatTable_rowreq_ctx *rec = 0;
  char* sysId = 0;
  stat::CommonPerformanceCounter* counter = 0;

  smeStatIndex = 0;  //ToDo
//#ifdef DEBUG
  fakeFillHashIfEmpty(h);
//#endif
  h.First();
  while ( h.Next(sysId, counter) )
  {
    smsc_log_debug(log, "smeStatTable_cache_load: h.Next %s", sysId ? sysId : "empty");
    rec = smeStatTable_allocate_rowreq_ctx();
    if (NULL == rec)
    {
      smsc_log_error(log, "smeStatTable_cache_load: memory allocation failed");
//      snmp_log(LOG_ERR, "memory allocation failed\n");
      continue;
//      return MFD_RESOURCE_UNAVAILABLE;
    }

    if( MFD_SUCCESS != smeStatTable_indexes_set(rec, smeStatIndex) )
    {
      smsc_log_error(log, "smeStatTable_cache_load: error setting index while loading smeStatTable data");
//      snmp_log(LOG_ERR, "error setting index while loading smeStatTable data.\n");
      smeStatTable_release_rowreq_ctx(rec);
      continue;
    }
    std::string idxStr = netsnmp_index2str(rec->oid_idx);
    std::string oidStr = oid2str(rec->oid_tmp, MAX_smeStatTable_IDX_LEN);
    smsc_log_debug(log, "smeStatTable_cache_load: smeStatTable_indexes_set(%d)=%s %s %d",
      smeStatIndex, idxStr.c_str(), oidStr.c_str(), rec->tbl_idx.smeStatIndex);

    ++smeStatIndex;  //ToDo

    if ( !fillRecord(rec, sysId, counter) )
    {
      smsc_log_error(log, "smeStatTable_cache_load fillRecord error");
      continue;
    }
    if ( container->insert_filter )
    {
      smsc_log_debug(log, "smeStatTable_cache_load: container insert_filter (%p)=%d",
        container->insert_filter, container->insert_filter(container, rec));
    }
    else
      smsc_log_debug(log, "smeStatTable_cache_load: container: NO insert_filter");

    int rc = CONTAINER_INSERT(container, rec);
    if ( 0 == rc )
    {
//      smsc_log_debug(log, "smeStatTable_cache_load CONTAINER_INSERT returns 0");
      ++recCount;
    }
    else
    {
      smsc_log_error(log, "smeStatTable_cache_load CONTAINER_INSERT returns(%d)", rc);
    }
    if (sysId) sysId = 0;
    if (counter) counter->clear();
  }

//  DEBUGMSGT(("verbose:smeStatTable:smeStatTable_cache_load", "inserted %d records\n", (int)recCount));
  smsc_log_debug(log, "smeStatTable_cache_load: inserted %d records, last [%s]", (int)recCount, sysId ? sysId : "empty");

  return MFD_SUCCESS;
}

/**
 * cache clean up
 *
 * @param container container with all current items
 *
 *  This optional callback is called prior to all
 *  item's being removed from the container. If you
 *  need to do any processing before that, do it here.
 *
 * @note
 *  The MFD helper will take care of releasing all the row contexts.
 *
 */
void smeStatTable_cache_free(netsnmp_container *container)
{
  smsc_log_debug(log, "smeStatTable_cache_free called");

    /*
     * TODO:380:M: Free smeStatTable cache.
     */
} /* smeStatTable_cache_free */


/**
 * prepare row for processing.
 *
 *  When the agent has located the row for a request, this function is
 *  called to prepare the row for processing. If you fully populated
 *  the data context during the index setup phase, you may not need to
 *  do anything.
 *
 * @param rowreq_ctx pointer to a context.
 *
 * @retval MFD_SUCCESS     : success.
 * @retval MFD_ERROR       : other error.
 */
int smeStatTable_row_prep( smeStatTable_rowreq_ctx *rowreq_ctx)
{
    DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_row_prep","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    /*
     * TODO:390:o: Prepare row for request.
     * If populating row data was delayed, this is the place to
     * fill in the row for this request.
     */

    return MFD_SUCCESS;
} /* smeStatTable_row_prep */


}}}
/** @} */
