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
//#include "logger/Logger.h"
#include "core/buffers/Hash.hpp"

#include "scag/stat/impl/Performance.h"

#include "scag/transport/smpp/base/SmppManager2.h"
#include "scag/snmp/SnmpUtil.h"


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

//smsc::logger::Logger* log;
//const char* containerName = "smeStatTableContainer";

bool fillRecord(smeStatTable_rowreq_ctx* rec, const char* sysId, stat::CommonPerformanceCounter* counter)
{
  if (!sysId)
    return false;
  if (!counter)
    return false;

  char smeStatSystemId[34];
  size_t smeStatSystemId_len = (strlen(sysId)<32) ? strlen(sysId) : 32;
  strncpy(&smeStatSystemId[0], sysId, smeStatSystemId_len);

  rec->data.smeStatSystemId_len = smeStatSystemId_len * sizeof(smeStatSystemId[0]);
  memcpy( rec->data.smeStatSystemId, smeStatSystemId, smeStatSystemId_len * sizeof(smeStatSystemId[0]) );

  uint64_t* pcnt = &counter->cntEvent[0]; // first 'ussd' counter, second 'sms' counter
  uint64_to_U64((*pcnt++ + *pcnt++), rec->data.smeStatAccepted);
  uint64_to_U64((*pcnt++ + *pcnt++), rec->data.smeStatRejected);
  uint64_to_U64((*pcnt++ + *pcnt++), rec->data.smeStatDelivered);
  uint64_to_U64((*pcnt++ + *pcnt++), rec->data.smeStatGwRejected);
  uint64_to_U64((*pcnt++ + *pcnt++), rec->data.smeStatFailed);
  uint64_to_U64((*pcnt++ + *pcnt++), rec->data.smeStatReceiptOk);
  uint64_to_U64((*pcnt++ + *pcnt++), rec->data.smeStatReceiptFailed);

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
  smsc_log_debug(log, "smeStatTable_init_data");
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
  smsc_log_debug(log, "smeStatTable_container_init");
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
    initConfigParams(cache->timeout, log);
} /* smeStatTable_container_init */


void smeStatTable_container_shutdown(netsnmp_container *container_ptr)
{
  smsc_log_debug(log, "smeStatTable_container_shutdown");
  DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_container_shutdown","called\n"));
  if (NULL == container_ptr) {
    snmp_log(LOG_ERR,"bad params to smeStatTable_container_shutdown\n");
    return;
  }
} /* smeStatTable_container_shutdown */

int fillNextCounter(netsnmp_container* container, const char* sysId, stat::CommonPerformanceCounter* counter)
{
  const char* sid = sysId ? sysId : noStatData;
  long smeStatIndex = 0;

  if (sysId) {
    try {
      smeStatIndex = sysId ? scag2::transport::smpp::SmppManager::Instance().getSmeIndex(sysId) : 0;
    }
    catch(...) {
      smsc_log_error(log, "smeStatTable_cache_load: error, SmppManager::getSmeIndex('%s') unavailable", sysId);
      return -4;
    }
  }

  int result = 0;
  smeStatTable_rowreq_ctx *rec = 0;

  do {
    rec = smeStatTable_allocate_rowreq_ctx();

    if (!rec) {
      smsc_log_error(log, "smeStatTable_cache_load: memory allocation failed");
      result = -1;
      break;
    }

    if( MFD_SUCCESS != smeStatTable_indexes_set(rec, smeStatIndex) ) {
      smsc_log_error(log, "smeStatTable_cache_load: error setting index while loading smeStatTable data");
      smeStatTable_release_rowreq_ctx(rec);
      result = -2;
      break;
    }

//    logIndexDebug(log, "smeStatTable", smeStatIndex, rec->tbl_idx.smeStatIndex, rec->oid_idx, rec->oid_tmp, MAX_smeStatTable_IDX_LEN);

    if ( !fillRecord(rec, sid, counter) ) {
      smsc_log_error(log, "smeStatTable_cache_load: fillRecord error");
      result = -3;
      break;
    }

    int rc = CONTAINER_INSERT(container, rec);
    if ( 0 != rc ) {
      smsc_log_error(log, "smeStatTable_cache_load: CONTAINER_INSERT(%s) returns(%d)", sysId, rc);
      result = rc;
      break;
    }
  } while (0);
  return result;
}

int loadHashToContainer(netsnmp_container* container, smsc::core::buffers::Hash<stat::CommonPerformanceCounter*>& h, int& recCount)
{
  char* sysId = 0;
  stat::CommonPerformanceCounter* counter = 0;

  smsc_log_debug(log, "loadHashToContainer '%s' hashSize %d", container->container_name, h.GetCount());

  h.First();
  while ( h.Next(sysId, counter) )
  {
    smsc_log_debug(log, "loadHashToContainer '%s' sysId '%s' cnt[0]=%lu", container->container_name, sysId, counter->cntEvent[0]);
    int result = fillNextCounter(container, (const char*)sysId, counter);
    if ( 0 == result )
    {
      ++recCount;
    }
    else if ( -1 == result )
    {
      return MFD_RESOURCE_UNAVAILABLE;
    }
    if (sysId) sysId = 0;
    if (counter) counter->clear();
  }
  return MFD_SUCCESS;
}

int smeStatTable_cache_load(netsnmp_container* container)
{
  smsc_log_debug(log, "smeStatTable_cache_load");
  int retCode = 0;
  int recCount = 0;
  if (container)
  {
    if ( !container->container_name )
      container->container_name = (char*)containerName;
//    smsc_log_debug(log, "smeStatTable_cache_load container %s", container->container_name);
  }
  else
  {
    smsc_log_error(log, "smeStatTable_cache_load error: container is NULL");
    return MFD_RESOURCE_UNAVAILABLE;
  }
  smsc::core::buffers::Hash<stat::CommonPerformanceCounter*>& h0 = getStatMan()->getCounters(0);
//  smsc_log_debug(log, "smeStatTable_cache_load: getCounters(0) ok, %d entries", h0.GetCount());

  retCode = loadHashToContainer(container, h0, recCount);
  if ( retCode == MFD_SUCCESS )
  {
    smsc::core::buffers::Hash<stat::CommonPerformanceCounter*>& h1 = getStatMan()->getCounters(1);
    retCode = loadHashToContainer(container, h1, recCount);
  }
  if ( 0 == recCount && fillEmptyData() )  // fill zero data if counters hash is empty
  {
    stat::CommonPerformanceCounter* counter = 0;
    smsc_log_debug(log, "smeStatTable_cache_load: no records, make fake counters");
    counter = new stat::CommonPerformanceCounter(stat::Counters::cntSmppSize);
    for ( int i=0; i<stat::Counters::cntSmppSize; ++i ) counter->cntEvent[i] = 0;
    counter->cntErrors.Insert(0,0);
    fillNextCounter(container, 0, counter);
    ++recCount;
  }
//  smsc_log_debug(log, "smeStatTable_cache_load: inserted %d records, retCode =%d", recCount, retCode);
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
void smeStatTable_cache_free(netsnmp_container *container)
{
  smsc_log_debug(log, "smeStatTable_cache_free");

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
  smsc_log_debug(log, "smeStatTable_row_prep");
//    DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_row_prep","called\n"));

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
