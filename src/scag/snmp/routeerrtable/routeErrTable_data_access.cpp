/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.12 $ of : mfd-data-access.m2c,v $
 *
 * $Id$
 */
/* standard Net-SNMP includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* include our parent header */
#include "routeErrTable.hpp"


#include "routeErrTable_data_access.hpp"

#include "util/debug.h"

#include "util/int.h"

#include "scag/transport/smpp/base/SmppManager2.h"
#include "scag/stat/impl/StatisticsManager.h"
#include "scag/stat/impl/StatCountersEnum.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"

#include "scag/config/base/ConfigManager2.h"
#include "scag/snmp/smestattable/smeStatTable_subagent.hpp"

#include "logger/Logger.h"

/** @defgroup data_access data_access: Routines to access data
 *
 * These routines are used to locate the data used to satisfy
 * requests.
 *
 * @{
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table routeErrTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * routeErrTable is subid 13 of msag.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.2.13, length: 9
*/


namespace scag2 {
namespace snmp {
namespace routeerrtable {

smsc::logger::Logger* log;
const char* containerName = "routeErrTableContainer";
const char* noStatData = "No-statistics-data";


/**
 * initialization for routeErrTable data access
 *
 * This function is called during startup to allow you to
 * allocate any resources you need for the data table.
 *
 * @param routeErrTable_reg
 *        Pointer to routeErrTable_registration
 *
 * @retval MFD_SUCCESS : success.
 * @retval MFD_ERROR   : unrecoverable error.
 */
int routeErrTable_init_data(routeErrTable_registration_ptr routeErrTable_reg)
{
//    DEBUGMSGTL(("verbose:routeErrTable:routeErrTable_init_data","called\n"));

    /*
     * TODO:303:o: Initialize routeErrTable data.
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

    log = smsc::logger::Logger::getInstance("snmp.rerr");
    return MFD_SUCCESS;
} /* routeErrTable_init_data */

/**
 * container-cached overview
 *
 */

/***********************************************************************
 *
 * cache
 *
 ***********************************************************************/
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
void routeErrTable_container_init(netsnmp_container **container_ptr_ptr,
                        netsnmp_cache *cache)
{
//    DEBUGMSGTL(("verbose:routeErrTable:routeErrTable_container_init","called\n"));

    if((NULL == cache) || (NULL == container_ptr_ptr)) {
      smsc_log_error(log, "bad params to routeErrTable_container_init");
//      snmp_log(LOG_ERR,"bad params to routeErrTable_container_init\n");
      return;
    }

    /*
     * For advanced users, you can use a custom container. If you
     * do not create one, one will be created for you.
     */
    *container_ptr_ptr = NULL;

    /*
     * TODO:345:A: Set up routeErrTable cache properties.
     *
     * Also for advanced users, you can set parameters for the
     * cache. Do not change the magic pointer, as it is used
     * by the MFD helper. To completely disable caching, set
     * cache->enabled to 0.
     */
    config::ConfigManager& cfg = config::ConfigManager::Instance();
    int cacheTimeout = 60;
    try {
      cacheTimeout = cfg.getConfig()->getInt("snmp.cacheTimeout");
    }
    catch (...) {
      __warning2__("Config parameter snmp.cacheTimeout not found, using default=%d", cacheTimeout);
    }
    cache->timeout = cacheTimeout; /* seconds */
} /* routeErrTable_container_init */

/**
 * load cache data
 *
 * TODO:350:M: Implement routeErrTable cache load
 *
 * @param container container to which items should be inserted
 *
 * @retval MFD_SUCCESS              : success.
 * @retval MFD_RESOURCE_UNAVAILABLE : Can't access data source
 * @retval MFD_ERROR                : other error.
 *
 *  This function is called to cache the index(es) (and data, optionally)
 *  for the every row in the data set.
 *
 * @remark
 *  While loading the cache, the only important thing is the indexes.
 *  If access to your data is cheap/fast (e.g. you have a pointer to a
 *  structure in memory), it would make sense to update the data here.
 *  If, however, the accessing the data invovles more work (e.g. parsing
 *  some other existing data, or peforming calculations to derive the data),
 *  then you can limit yourself to setting the indexes and saving any
 *  information you will need later. Then use the saved information in
 *  routeErrTable_row_prep() for populating data.
 *
 * @note
 *  If you need consistency between rows (like you want statistics
 *  for each row to be from the same time frame), you should set all
 *  data here.
 *
 */


void uint64_to_U64(uint64_t val1,U64& val2)
{
  val2.high=(val1>>32)&0xffffffffUL;
  val2.low=val1&0xffffffffUL;
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

int fillNextCounter(netsnmp_container* container, const char* sysId, long errIndex, stat::CommonPerformanceCounter* counter)
{
  int result = 0;
  const char* sid = sysId ? sysId : noStatData;
  int       errCode = 0;
  uint64_t  errCount = 0;
  routeErrTable_rowreq_ctx* rec = 0;

  for(scag2::stat::IntHash<uint64_t>::Iterator iter = counter->cntErrors.First(); iter.Next(errCode, errCount); )
  {
//    smsc_log_debug(log, "routeErrTable_cache_load: %s(%d) Next %d %lld", sid, errIndex, errCode, errCount);

    if (!errCount) continue;

    rec = routeErrTable_allocate_rowreq_ctx(NULL);
    if (NULL == rec)
    {
      smsc_log_error(log, "routeErrTable_cache_load: memory allocation failed");
      return -1;
    }

    if( MFD_SUCCESS != routeErrTable_indexes_set(rec, errIndex, errCode) )
    {
      smsc_log_error(log, "routeErrTable_cache_load: error setting index while loading routeErrTable data");
      routeErrTable_release_rowreq_ctx(rec);
      continue;
    }

    std::string idxStr = netsnmp_index2str(rec->oid_idx);
    std::string oidStr = oid2str(rec->oid_tmp, MAX_routeErrTable_IDX_LEN);
//    smsc_log_debug(log, "routeErrTable_cache_load: routeErrTable_indexes_set(%d)=%s %s %d",
//        errIndex, idxStr.c_str(), oidStr.c_str(), rec->tbl_idx.routeErrIndex);

    if ((NULL == rec->data.routeErrId) || strlen(sid) > sizeof(rec->data.routeErrId))
    {
      smsc_log_error(log, "routeErrTable_cache_load: not enough space for value");
      snmp_log(LOG_ERR,"not enough space for value\n");
      return -2;
    }

    rec->data.routeErrId_len = strlen(sid);
    memcpy( rec->data.routeErrId, sid, rec->data.routeErrId_len+1 );

    uint64_to_U64(errCount, rec->data.routeErrCount);

    int rc = CONTAINER_INSERT(container, rec);
    if ( 0 == rc )
      ++result;
    else
    {
      smsc_log_error(log, "routeErrTable_cache_load CONTAINER_INSERT returns(%d)", rc);
      return -3;
    }
  }
  return result;
}

int loadHashToContainer(netsnmp_container* container, smsc::core::buffers::Hash<stat::CommonPerformanceCounter*>& h, int& recCount)
{
  long      errIndex = 0;
  int       errCode = 0;
  uint64_t  errCount = 0;

  routeErrTable_rowreq_ctx* rec = 0;
  char* routeId = 0;
  stat::CommonPerformanceCounter* cs;

  h.First();
  while ( h.Next(routeId, cs) )
  {
    ++errIndex;
    int result = fillNextCounter(container, (const char*)routeId, errIndex, cs);
    if ( result >= 0 )
    {
      recCount += result;
    }
    else if ( -1 == result )
    {
      return MFD_RESOURCE_UNAVAILABLE;
    }
    else
    {
      return MFD_ERROR;
    }
  }
  return MFD_SUCCESS;
}

int routeErrTable_cache_load(netsnmp_container* container)
{
  int retCode = 0;
  int recCount = 0;
  if (container)
  {
    if ( !container->container_name )
      container->container_name = (char*)containerName;
//    smsc_log_debug(log, "routeErrTable_cache_load container %s", container->container_name);
  }
  else
  {
    smsc_log_error(log, "routeErrTable_cache_load error: container is NULL");
    return MFD_RESOURCE_UNAVAILABLE;
  }

  smsc::core::buffers::Hash<stat::CommonPerformanceCounter*>& h0 = scag2::snmp::smestattable::SmeStatTableSubagent::getStatMan()->getRouteCounters();

  retCode = loadHashToContainer(container, h0, recCount);
  if ( 0 == recCount )  // fill zero data if counters hash is empty
  {
    stat::CommonPerformanceCounter* counter = 0;
    smsc_log_debug(log, "routeErrTable_cache_load: no records, make fake counters");
    counter = new stat::CommonPerformanceCounter(stat::Counters::cntSmppSize);
    for ( int i=0; i<stat::Counters::cntSmppSize; ++i ) counter->cntEvent[i] = 0;
    counter->cntErrors.Insert(0, 1);
    fillNextCounter(container, 0, 1, counter);
    ++recCount;
  }

  smsc_log_debug(log, "routeErrTable_cache_load: inserted %d records, retCode=%d", recCount, retCode);
  return retCode;
} /* routeErrTable_cache_load */


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
void routeErrTable_cache_free(netsnmp_container *container)
{
    DEBUGMSGTL(("verbose:routeErrTable:routeErrTable_cache_free","called\n"));

    /*
     * TODO:380:M: Free routeErrTable cache.
     */
} /* routeErrTable_cache_free */

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
int routeErrTable_row_prep( routeErrTable_rowreq_ctx *rowreq_ctx)
{
    DEBUGMSGTL(("verbose:routeErrTable:routeErrTable_row_prep","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    /*
     * TODO:390:o: Prepare row for request.
     * If populating row data was delayed, this is the place to
     * fill in the row for this request.
     */

    return MFD_SUCCESS;
} /* routeErrTable_row_prep */


}}}

/** @} */
