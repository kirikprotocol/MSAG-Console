/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 12088 $ of $
 *
 * $Id$
 *
 * @file routeStatTable_data_get.h
 *
 * @addtogroup get
 *
 * Prototypes for get functions
 *
 * @{
 */
#ifndef ROUTEUSSDTABLE_DATA_GET_H
#define ROUTEUSSDTABLE_DATA_GET_H

namespace scag2 {
namespace snmp {
namespace routeussdtable {

/* *********************************************************************
 * GET function declarations
 */

/* *********************************************************************
 * GET Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table routeStatTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * EYELINE-MSAG-MIB::routeStatTable is subid 10 of msag.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.2.12, length: 9
*/
    /*
     * indexes
     */

    int routeStatRouteId_get( routeStatTable_rowreq_ctx *rowreq_ctx, char **routeStatRouteId_val_ptr_ptr, size_t *routeStatRouteId_val_ptr_len_ptr );
    int routeStatAccepted_get( routeStatTable_rowreq_ctx *rowreq_ctx, U64 * routeStatAccepted_val_ptr );
    int routeStatRejected_get( routeStatTable_rowreq_ctx *rowreq_ctx, U64 * routeStatRejected_val_ptr );
    int routeStatDelivered_get( routeStatTable_rowreq_ctx *rowreq_ctx, U64 * routeStatDelivered_val_ptr );
    int routeStatGwRejected_get( routeStatTable_rowreq_ctx *rowreq_ctx, U64 * routeStatGwRejected_val_ptr );
    int routeStatFailed_get( routeStatTable_rowreq_ctx *rowreq_ctx, U64 * routeStatFailed_val_ptr );
    int routeStatReceiptOk_get( routeStatTable_rowreq_ctx *rowreq_ctx, U64 * routeStatReceiptOk_val_ptr );
    int routeStatReceiptFailed_get( routeStatTable_rowreq_ctx *rowreq_ctx, U64 * routeStatReceiptFailed_val_ptr );


  int routeStatTable_indexes_set_tbl_idx(routeStatTable_mib_index *tbl_idx, long routeStatIndex_val);
  int routeStatTable_indexes_set(routeStatTable_rowreq_ctx *rowreq_ctx, long routeStatIndex_val);

  int routeStatIndex_map(long *mib_routeStatIndex_val_ptr, long raw_routeStatIndex_val);
  int routeStatRouteId_map(char **mib_routeStatRouteId_val_ptr_ptr, size_t *mib_routeStatRouteId_val_ptr_len_ptr,
      char *raw_routeStatRouteId_val_ptr, size_t raw_routeStatRouteId_val_ptr_len, int allow_realloc);
  int routeStatAccepted_map(U64 *mib_routeStatAccepted_val_ptr, U64 raw_routeStatAccepted_val);
  int routeStatRejected_map(U64 *mib_routeStatRejected_val_ptr, U64 raw_routeStatRejected_val);
  int routeStatDelivered_map(U64 *mib_routeStatDelivered_val_ptr, U64 raw_routeStatDelivered_val);
  int routeStatGwRejected_map(U64 *mib_routeStatGwRejected_val_ptr, U64 raw_routeStatGwRejected_val);
  int routeStatFailed_map(U64 *mib_routeStatFailed_val_ptr, U64 raw_routeStatFailed_val);

}}}

#endif /* ROUTEUSSDTABLE_DATA_GET_H */
/** @} */
