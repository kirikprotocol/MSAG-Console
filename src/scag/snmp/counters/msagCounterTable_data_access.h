/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 14170 $ of $
 *
 * $Id$
 */
#ifndef MSAGCOUNTERTABLE_DATA_ACCESS_H
#define MSAGCOUNTERTABLE_DATA_ACCESS_H

#ifdef __cplusplus
extern "C" {
#endif


/* *********************************************************************
 * function declarations
 */

/* *********************************************************************
 * Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table msagCounterTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * EYELINE-MSAG-MIB::msagCounterTable is subid 4 of msag.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.2.4, length: 9
*/


int msagCounterTable_init_data(msagCounterTable_registration_ptr msagCounterTable_reg);


    /*
     * TODO:180:o: Review msagCounterTable cache timeout.
     * The number of seconds before the cache times out
     */
// #define MSAGCOUNTERTABLE_CACHE_TIMEOUT   60

void msagCounterTable_container_init(netsnmp_container **container_ptr_ptr,
                             netsnmp_cache *cache);
int msagCounterTable_cache_load(netsnmp_container *container);
void msagCounterTable_cache_free(netsnmp_container *container);

int msagCounterTable_row_prep( msagCounterTable_rowreq_ctx *rowreq_ctx);

#ifdef __cplusplus
}
#endif

#endif /* MSAGCOUNTERTABLE_DATA_ACCESS_H */
