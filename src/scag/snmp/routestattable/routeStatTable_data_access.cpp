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
#include "routeStatTable.hpp"
#include "routeStatTable_data_access.hpp"
#include "routeStatTable_subagent.hpp"
#include "util/int.h"

#include "scag/stat/impl/StatisticsManager.h"
#include "scag/stat/impl/StatCountersEnum.hpp"
#include "logger/Logger.h"
#include "core/buffers/Hash.hpp"

#include "scag/stat/impl/Performance.h"

#include "scag/config/base/ConfigManager2.h"
#include "scag/snmp/smestattable/smeStatTable_subagent.hpp"


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
 *** Table routeStatTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * EYELINE-MSAG-MIB::routeStatTable is subid 12 of msag.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.2.12, length: 9
*/

namespace scag2 {
namespace snmp {
namespace routestattable {

smsc::logger::Logger* log;
const char* containerName = "routeStatTableContainer";
const char* noStatData = "No-statistics-data";
bool fillEmptyData = false;

void uint64_to_U64(uint64_t val1, U64& val2)
{
  val2.high = (val1>>32)&0xffffffffUL;
  val2.low = val1&0xffffffffUL;
}

bool fillRecord(routeStatTable_rowreq_ctx* rec, const char* routeId, stat::CommonPerformanceCounter* counter)
{
//  smsc_log_debug(log, "fillRecord() routeId %s count %d", routeId?routeId:"empty", counter?counter->count:0);
  if (!routeId)
    return false;
  if (!counter)
    return false;

  char routeStatId[34];
  size_t routeStatId_len = (strlen(routeId)<32) ? strlen(routeId) : 32;
  strncpy(&routeStatId[0], routeId, routeStatId_len);

/*
  if (
       (NULL == rec->data.routeStatRouteId)
       || (rec->data.routeStatRouteId_len < (routeStatId_len * sizeof(routeStatId[0])))
     )
  {
    snmp_log(LOG_ERR,"not enough space for value\n");
    return false;
  }
*/

  rec->data.routeStatRouteId_len = routeStatId_len * sizeof(routeStatId[0]);
  memcpy( rec->data.routeStatRouteId, routeStatId, routeStatId_len * sizeof(routeStatId[0]) );

  uint64_to_U64(counter->cntEvent[stat::Counters::cntAccepted],      rec->data.routeStatAccepted);
  uint64_to_U64(counter->cntEvent[stat::Counters::cntRejected],      rec->data.routeStatRejected);
  uint64_to_U64(counter->cntEvent[stat::Counters::cntDelivered],     rec->data.routeStatDelivered);
  uint64_to_U64(counter->cntEvent[stat::Counters::cntGw_Rejected],   rec->data.routeStatGwRejected);
  uint64_to_U64(counter->cntEvent[stat::Counters::cntFailed],        rec->data.routeStatFailed);
  uint64_to_U64(counter->cntEvent[stat::Counters::cntRecieptOk],     rec->data.routeStatReceiptOk);
  uint64_to_U64(counter->cntEvent[stat::Counters::cntRecieptFailed], rec->data.routeStatReceiptFailed);

//  smsc_log_debug(log, "fillRecord() %s OK", routeId);
  return true;
}


/**
 * initialization for routeStatTable data access
 *
 * This function is called during startup to allow you to
 * allocate any resources you need for the data table.
 *
 * @param routeStatTable_reg
 *        Pointer to routeStatTable_registration
 *
 * @retval MFD_SUCCESS : success.
 * @retval MFD_ERROR   : unrecoverable error.
 */
int routeStatTable_init_data(routeStatTable_registration_ptr routeStatTable_reg)
{
    DEBUGMSGTL(("verbose:routeStatTable:routeStatTable_init_data","called\n"));

    /*
     * TODO:303:o: Initialize routeStatTable data.
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

    log = smsc::logger::Logger::getInstance("snmp.rstat");
    return MFD_SUCCESS;
} /* routeStatTable_init_data */

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
void routeStatTable_container_init(netsnmp_container **container_ptr_ptr, netsnmp_cache *cache)
{
    DEBUGMSGTL(("verbose:routeStatTable:routeStatTable_container_init","called\n"));
    
    if((NULL == cache) || (NULL == container_ptr_ptr)) {
//    if (NULL == container_ptr_ptr) {
      snmp_log(LOG_ERR, "bad container param to routeStatTable_container_init\n");
      return;
    }

    /*
     * For advanced users, you can use a custom container. If you
     * do not create one, one will be created for you.
     */
    *container_ptr_ptr = NULL;

    /*
     * TODO:345:A: Set up routeStatTable cache properties.
     *
     * Also for advanced users, you can set parameters for the
     * cache. Do not change the magic pointer, as it is used
     * by the MFD helper. To completely disable caching, set
     * cache->enabled to 0.
     */
//    cache->timeout = ROUTESTATTABLE_CACHE_TIMEOUT; /* seconds */
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
    try {
      fillEmptyData = config::ConfigManager::Instance().getConfig()->getBool("snmp.fillEmptyData");
    }
    catch (...) {
      smsc_log_info(log, "Config parameter snmp.fillEmptyData not found, using default=%s", fillEmptyData?"true":"false");
      snmp_log(LOG_ERR, "Config parameter snmp.fillEmptyData not found, using default=%s\n", fillEmptyData?"true":"false");
    }
} /* routeStatTable_container_init */

void routeStatTable_container_shutdown(netsnmp_container *container_ptr)
{
  DEBUGMSGTL(("verbose:routeStatTable:routeStatTable_container_shutdown","called\n"));
  if (NULL == container_ptr) {
    snmp_log(LOG_ERR,"bad params to routeStatTable_container_shutdown\n");
    return;
  }
} /* routeStatTable_container_shutdown */

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

int fillNextCounter(netsnmp_container* container, const char* routeId, long routeStatIndex, stat::CommonPerformanceCounter* counter)
{
  const char* sid = routeId ? routeId : noStatData;
//  smsc_log_debug(log, "routeStatTable_cache_load: h.Next %s (%d)", sid, routeStatIndex);

  routeStatTable_rowreq_ctx *rec = 0;

  rec = routeStatTable_allocate_rowreq_ctx();
  if (NULL == rec)
  {
    smsc_log_error(log, "routeStatTable_cache_load: memory allocation failed");
    return -1;
  }

  if( MFD_SUCCESS != routeStatTable_indexes_set(rec, routeStatIndex) )
  {
    smsc_log_error(log, "routeStatTable_cache_load: error setting index while loading routeStatTable data");
    routeStatTable_release_rowreq_ctx(rec);
    return -2;
  }
  std::string idxStr = netsnmp_index2str(rec->oid_idx);
  std::string oidStr = oid2str(rec->oid_tmp, MAX_routeStatTable_IDX_LEN);
//  smsc_log_debug(log, "routeStatTable_cache_load: routeStatTable_indexes_set(%d)=%s %s %d",
//      routeStatIndex, idxStr.c_str(), oidStr.c_str(), rec->tbl_idx.routeStatIndex);

  if ( !fillRecord(rec, sid, counter) )
  {
    smsc_log_error(log, "routeStatTable_cache_load fillRecord error");
    return -3;
  }

  int rc = CONTAINER_INSERT(container, rec);
  if ( 0 != rc )
    smsc_log_error(log, "routeStatTable_cache_load CONTAINER_INSERT returns(%d)", rc);
  return rc;
}

int loadHashToContainer(netsnmp_container* container, smsc::core::buffers::Hash<stat::CommonPerformanceCounter*>& h, int& recCount)
{
  long   routeStatIndex = 0;
  char* routeId = 0;
  stat::CommonPerformanceCounter* counter = 0;

  h.First();
  while ( h.Next(routeId, counter) )
  {
    routeStatIndex++;
    int result = fillNextCounter(container, (const char*)routeId, routeStatIndex, counter);
    if ( 0 == result )
    {
      ++recCount;
    }
    else if ( -1 == result )
    {
      return MFD_RESOURCE_UNAVAILABLE;
    }
    if (routeId) routeId = 0;
    if (counter) counter->clear();
  }
  return MFD_SUCCESS;
}

int routeStatTable_cache_load(netsnmp_container* container)
{
  int retCode = 0;
  int recCount = 0;
  if (container)
  {
    if ( !container->container_name )
      container->container_name = (char*)containerName;
//    smsc_log_debug(log, "routeStatTable_cache_load container %s", container->container_name);
  }
  else
  {
    smsc_log_error(log, "routeStatTable_cache_load error: container is NULL");
    return MFD_RESOURCE_UNAVAILABLE;
  }
  smsc::core::buffers::Hash<stat::CommonPerformanceCounter*>& h0 = scag2::snmp::smestattable::SmeStatTableSubagent::getStatMan()->getRouteCounters();
  smsc_log_debug(log, "routeStatTable_cache_load: getCounters(0) ok, %d entries", h0.GetCount());

  retCode = loadHashToContainer(container, h0, recCount);
  if ( 0 == recCount && fillEmptyData )  // fill zero data if counters hash is empty
  {
    stat::CommonPerformanceCounter* counter = 0;
    smsc_log_debug(log, "routeStatTable_cache_load: no records, make fake counters");
    counter = new stat::CommonPerformanceCounter(stat::Counters::cntSmppSize);
    for ( int i=0; i<stat::Counters::cntSmppSize; ++i ) counter->cntEvent[i] = 0;
    counter->cntErrors.Insert(0,0);
    fillNextCounter(container, 0, 1, counter);
    ++recCount;
  }
  smsc_log_debug(log, "routeStatTable_cache_load: inserted %d records, retCode=%d", recCount, retCode);
  return retCode;
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
void routeStatTable_cache_free(netsnmp_container *container)
{
//  smsc_log_debug(log, "routeStatTable_cache_free called");

    /*
     * TODO:380:M: Free routeStatTable cache.
     */
} /* routeStatTable_cache_free */


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
int routeStatTable_row_prep( routeStatTable_rowreq_ctx *rowreq_ctx)
{
//    DEBUGMSGTL(("verbose:routeStatTable:routeStatTable_row_prep","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    /*
     * TODO:390:o: Prepare row for request.
     * If populating row data was delayed, this is the place to
     * fill in the row for this request.
     */

    return MFD_SUCCESS;
} /* routeStatTable_row_prep */


}}}
/** @} */
