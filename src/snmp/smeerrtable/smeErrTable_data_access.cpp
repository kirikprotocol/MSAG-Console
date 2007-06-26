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
#include "smeErrTable.hpp"


#include "smeErrTable_data_access.hpp"

#include "snmp/smestattable/smeStatTable.hpp"

#include "smeman/smeman.h"
#include "stat/SmeStats.hpp"
#include "util/config/Manager.h"

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
 *** Table smeErrTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * smeErrTable is subid 11 of smsc.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.1.11, length: 9
*/

namespace smsc{
namespace snmp{
namespace smeerrtable{


/**
 * initialization for smeErrTable data access
 *
 * This function is called during startup to allow you to
 * allocate any resources you need for the data table.
 *
 * @param smeErrTable_reg
 *        Pointer to smeErrTable_registration
 *
 * @retval MFD_SUCCESS : success.
 * @retval MFD_ERROR   : unrecoverable error.
 */
int
smeErrTable_init_data(smeErrTable_registration_ptr smeErrTable_reg)
{
    DEBUGMSGTL(("verbose:smeErrTable:smeErrTable_init_data","called\n"));

    /*
     * TODO:303:o: Initialize smeErrTable data.
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
} /* smeErrTable_init_data */

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
void
smeErrTable_container_init(netsnmp_container **container_ptr_ptr,
                        netsnmp_cache *cache)
{
    DEBUGMSGTL(("verbose:smeErrTable:smeErrTable_container_init","called\n"));

    if((NULL == cache) || (NULL == container_ptr_ptr)) {
        snmp_log(LOG_ERR,"bad params to smeErrTable_container_init\n");
        return;
    }

    /*
     * For advanced users, you can use a custom container. If you
     * do not create one, one will be created for you.
     */
    *container_ptr_ptr = NULL;

    /*
     * TODO:345:A: Set up smeErrTable cache properties.
     *
     * Also for advanced users, you can set parameters for the
     * cache. Do not change the magic pointer, as it is used
     * by the MFD helper. To completely disable caching, set
     * cache->enabled to 0.
     */
    int to=60;
    try{
      to=smsc::util::config::Manager::getInstance().getInt("snmp.cacheTimeout");
    }
    catch(...)
    {
      __warning2__("Config parameter snmp.cacheTimeout not found, using default=%d",to);
    }
    cache->timeout = to; /* seconds */
} /* smeErrTable_container_init */

/**
 * load cache data
 *
 * TODO:350:M: Implement smeErrTable cache load
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
 *  smeErrTable_row_prep() for populating data.
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

int
smeErrTable_cache_load(netsnmp_container *container)
{
    smeErrTable_rowreq_ctx *rowreq_ctx;
    size_t                 count = 0;

    /*
     * this example code is based on a data source that is a
     * text file to be read and parsed.
     */
    U64    smeErrCount;
    long   smeErrIndex;
    long   smeErrCode;

    DEBUGMSGTL(("verbose:smeErrTable:smeErrTable_cache_load","called\n"));

    smsc::smeman::SmeManager* smeman=smsc::snmp::smestattable::getSmeMan();
    smsc::stat::SmeStats* smestat=smsc::snmp::smestattable::getSmeStats();

    int cnt=smeman->getSmesCount();
    smsc::smeman::SmeInfo info;


    for(smeErrIndex=0;smeErrIndex<cnt;smeErrIndex++)
    {
      info=smeman->getSmeInfo(smeErrIndex);
      if(info.disabled)continue;
      smsc::stat::SmeStats::ErrCntVector errors;
      smestat->getErrors(smeErrIndex,errors);
      for(smeErrCode=0;smeErrCode<errors.size();smeErrCode++)
      {
        if(errors[smeErrCode]==0)continue;
        uint64_to_U64(errors[smeErrCode],smeErrCount);

        /*
         * TODO:352:M: |   |-> set indexes in new smeErrTable rowreq context.
         * data context will be set from the param (unless NULL,
         *      in which case a new data context will be allocated)
         */
        rowreq_ctx = smeErrTable_allocate_rowreq_ctx(NULL);
        if (NULL == rowreq_ctx)
        {
          snmp_log(LOG_ERR, "memory allocation failed\n");
          return MFD_RESOURCE_UNAVAILABLE;
        }
        if (MFD_SUCCESS != smeErrTable_indexes_set(rowreq_ctx
                                                   , smeErrIndex
                                                   , smeErrCode
                                                  ))
        {
          snmp_log(LOG_ERR,"error setting index while loading "
                   "smeErrTable cache.\n");
          smeErrTable_release_rowreq_ctx(rowreq_ctx);
          continue;
        }

        /*
         * TODO:352:r: |   |-> populate smeErrTable data context.
         * Populate data context here. (optionally, delay until row prep)
         */
        /*
         * TRANSIENT or semi-TRANSIENT data:
         * copy data or save any info needed to do it in row_prep.
         */
        /*
         * setup/save data for smeErrSystemId
         * smeErrSystemId(3)/DisplayString/ASN_OCTET_STR/char(char)//L/A/w/e/R/d/H
         */
        /*
         * TODO:246:r: |-> Define smeErrSystemId mapping.
         * Map values between raw/native values and MIB values
         *
         * if(MFD_SUCCESS !=
         *    smeErrSystemId_map(&rowreq_ctx->data.smeErrSystemId, &rowreq_ctx->data.smeErrSystemId_len,
         *                smeErrSystemId, smeErrSystemId_len, 0)) {
         *    return MFD_ERROR;
         * }
         */
        /*
         * make sure there is enough space for smeErrSystemId data
         */
        if ((NULL == rowreq_ctx->data.smeErrSystemId) ||
             info.systemId.length()>sizeof(rowreq_ctx->data.smeErrSystemId))
        {
          snmp_log(LOG_ERR,"not enough space for value\n");
          return MFD_ERROR;
        }
        rowreq_ctx->data.smeErrSystemId_len=info.systemId.length();
        memcpy( rowreq_ctx->data.smeErrSystemId, info.systemId.c_str(), rowreq_ctx->data.smeErrSystemId_len+1 );

        /*
         * setup/save data for smeErrCount
         * smeErrCount(4)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
        /*
         * TODO:246:r: |-> Define smeErrCount mapping.
         * Map values between raw/native values and MIB values
         *
         * Integer based value can usually just do a direct copy.
         */
        rowreq_ctx->data.smeErrCount.high = smeErrCount.high;
        rowreq_ctx->data.smeErrCount.low = smeErrCount.low;


        /*
         * insert into table container
         */
        CONTAINER_INSERT(container, rowreq_ctx);
        ++count;
      }
    }


    DEBUGMSGT(("verbose:smeErrTable:smeErrTable_cache_load",
               "inserted %d records\n", count));

    return MFD_SUCCESS;
} /* smeErrTable_cache_load */

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
void
smeErrTable_cache_free(netsnmp_container *container)
{
    DEBUGMSGTL(("verbose:smeErrTable:smeErrTable_cache_free","called\n"));

    /*
     * TODO:380:M: Free smeErrTable cache.
     */
} /* smeErrTable_cache_free */

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
int smeErrTable_row_prep( smeErrTable_rowreq_ctx *rowreq_ctx)
{
    DEBUGMSGTL(("verbose:smeErrTable:smeErrTable_row_prep","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    /*
     * TODO:390:o: Prepare row for request.
     * If populating row data was delayed, this is the place to
     * fill in the row for this request.
     */

    return MFD_SUCCESS;
} /* smeErrTable_row_prep */


}//smeerrtable
}//snmp
}//smsc

/** @} */
