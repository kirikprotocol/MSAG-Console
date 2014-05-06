/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 12088 $ of $
 *
 * $Id$
 *
 * @file smeStatTable_data_get.h
 *
 * @addtogroup get
 *
 * Prototypes for get functions
 *
 * @{
 */
#ifndef SMESTATTABLE_DATA_GET_H
#define SMESTATTABLE_DATA_GET_H

#ifdef __cplusplus
extern "C" {
#endif

/* *********************************************************************
 * GET function declarations
 */

/* *********************************************************************
 * GET Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table smeStatTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * SIBINCO-SMSC-MIB::smeStatTable is subid 10 of smsc.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.1.10, length: 9
*/
    /*
     * indexes
     */

    int smeStatSystemId_get( smeStatTable_rowreq_ctx *rowreq_ctx, char **smeStatSystemId_val_ptr_ptr, size_t *smeStatSystemId_val_ptr_len_ptr );
    int smeStatAccepted_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatAccepted_val_ptr );
    int smeStatRejected_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatRejected_val_ptr );
    int smeStatDelivered_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatDelivered_val_ptr );
    int smeStatFailed_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatFailed_val_ptr );
    int smeStatTemporal_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatTemporal_val_ptr );
    int smeStatRetried_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatRetried_val_ptr );


int smeStatTable_indexes_set_tbl_idx(smeStatTable_mib_index *tbl_idx, long smeStatIndex_val);
int smeStatTable_indexes_set(smeStatTable_rowreq_ctx *rowreq_ctx, long smeStatIndex_val);




#ifdef __cplusplus
}
#endif

#endif /* SMESTATTABLE_DATA_GET_H */
/** @} */
