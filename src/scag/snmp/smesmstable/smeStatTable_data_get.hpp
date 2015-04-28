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

namespace scag2 {
namespace snmp {
namespace smesmstable {

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
 * EYELINE-MSAG-MIB::smeStatTable is subid 10 of msag.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.2.10, length: 9
*/
    /*
     * indexes
     */

    int smeStatSystemId_get( smeStatTable_rowreq_ctx *rowreq_ctx, char **smeStatSystemId_val_ptr_ptr, size_t *smeStatSystemId_val_ptr_len_ptr );
    int smeStatAccepted_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatAccepted_val_ptr );
    int smeStatRejected_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatRejected_val_ptr );
    int smeStatDelivered_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatDelivered_val_ptr );
    int smeStatGwRejected_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatGwRejected_val_ptr );
    int smeStatFailed_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatFailed_val_ptr );
    int smeStatReceiptOk_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatReceiptOk_val_ptr );
    int smeStatReceiptFailed_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatReceiptFailed_val_ptr );


  int smeStatTable_indexes_set_tbl_idx(smeStatTable_mib_index *tbl_idx, long smeStatIndex_val);
  int smeStatTable_indexes_set(smeStatTable_rowreq_ctx *rowreq_ctx, long smeStatIndex_val);

  int smeStatIndex_map(long *mib_smeStatIndex_val_ptr, long raw_smeStatIndex_val);
  int smeStatSystemId_map(char **mib_smeStatSystemId_val_ptr_ptr, size_t *mib_smeStatSystemId_val_ptr_len_ptr,
      char *raw_smeStatSystemId_val_ptr, size_t raw_smeStatSystemId_val_ptr_len, int allow_realloc);
  int smeStatAccepted_map(U64 *mib_smeStatAccepted_val_ptr, U64 raw_smeStatAccepted_val);
  int smeStatRejected_map(U64 *mib_smeStatRejected_val_ptr, U64 raw_smeStatRejected_val);
  int smeStatDelivered_map(U64 *mib_smeStatDelivered_val_ptr, U64 raw_smeStatDelivered_val);
  int smeStatGwRejected_map(U64 *mib_smeStatGwRejected_val_ptr, U64 raw_smeStatGwRejected_val);
  int smeStatFailed_map(U64 *mib_smeStatFailed_val_ptr, U64 raw_smeStatFailed_val);

}}}

#endif /* SMESTATTABLE_DATA_GET_H */
/** @} */
