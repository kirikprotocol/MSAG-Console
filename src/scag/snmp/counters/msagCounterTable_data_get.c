/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 12088 $ of $ 
 *
 * $Id$
 */
/* standard Net-SNMP includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* include our parent header */
#include "msagCounterTable.h"


/** @defgroup data_get data_get: Routines to get data
 *
 * TODO:230:M: Implement msagCounterTable get routines.
 * TODO:240:M: Implement msagCounterTable mapping routines (if any).
 *
 * These routine are used to get the value for individual objects. The
 * row context is passed, along with a pointer to the memory where the
 * value should be copied.
 *
 * @{
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

/* ---------------------------------------------------------------------
 * TODO:200:r: Implement msagCounterTable data context functions.
 */


/**
 * set mib index(es)
 *
 * @param tbl_idx mib index structure
 * @param msagCounterIndex_val
 *
 * @retval MFD_SUCCESS     : success.
 * @retval MFD_ERROR       : other error.
 *
 * @remark
 *  This convenience function is useful for setting all the MIB index
 *  components with a single function call. It is assume that the C values
 *  have already been mapped from their native/rawformat to the MIB format.
 */
int
msagCounterTable_indexes_set_tbl_idx(msagCounterTable_mib_index *tbl_idx, long msagCounterIndex_val)
{
    DEBUGMSGTL(("verbose:msagCounterTable:msagCounterTable_indexes_set_tbl_idx","called\n"));

    /* msagCounterIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/R/d/h */
    tbl_idx->msagCounterIndex = msagCounterIndex_val;
    

    return MFD_SUCCESS;
} /* msagCounterTable_indexes_set_tbl_idx */

/**
 * @internal
 * set row context indexes
 *
 * @param reqreq_ctx the row context that needs updated indexes
 *
 * @retval MFD_SUCCESS     : success.
 * @retval MFD_ERROR       : other error.
 *
 * @remark
 *  This function sets the mib indexs, then updates the oid indexs
 *  from the mib index.
 */
int
msagCounterTable_indexes_set(msagCounterTable_rowreq_ctx *rowreq_ctx, long msagCounterIndex_val)
{
    DEBUGMSGTL(("verbose:msagCounterTable:msagCounterTable_indexes_set","called\n"));

    if(MFD_SUCCESS != msagCounterTable_indexes_set_tbl_idx(&rowreq_ctx->tbl_idx
                                   , msagCounterIndex_val
           ))
        return MFD_ERROR;

    /*
     * convert mib index to oid index
     */
    rowreq_ctx->oid_idx.len = sizeof(rowreq_ctx->oid_tmp) / sizeof(oid);
    if(0 != msagCounterTable_index_to_oid(&rowreq_ctx->oid_idx,
                                    &rowreq_ctx->tbl_idx)) {
        return MFD_ERROR;
    }

    return MFD_SUCCESS;
} /* msagCounterTable_indexes_set */


/*---------------------------------------------------------------------
 * EYELINE-MSAG-MIB::msagCounterEntry.msagCounterName
 * msagCounterName is subid 2 of msagCounterEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.26757.2.4.1.2
 * Description:
MSAG Counter Name.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 1      hashint   1
 *   settable   0
 *   hint: 255a
 *
 * Ranges:  0 - 48;
 *
 * Its syntax is DisplayString (based on perltype OCTETSTR)
 * The net-snmp type is ASN_OCTET_STR. The C type decl is char (char)
 * This data type requires a length.  (Max 48)
 */
/**
 * Extract the current value of the msagCounterName data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param msagCounterName_val_ptr_ptr
 *        Pointer to storage for a char variable
 * @param msagCounterName_val_ptr_len_ptr
 *        Pointer to a size_t. On entry, it will contain the size (in bytes)
 *        pointed to by msagCounterName.
 *        On exit, this value should contain the data size (in bytes).
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
*
 * @note If you need more than (*msagCounterName_val_ptr_len_ptr) bytes of memory,
 *       allocate it using malloc() and update msagCounterName_val_ptr_ptr.
 *       <b>DO NOT</b> free the previous pointer.
 *       The MFD helper will release the memory you allocate.
 *
 * @remark If you call this function yourself, you are responsible
 *         for checking if the pointer changed, and freeing any
 *         previously allocated memory. (Not necessary if you pass
 *         in a pointer to static memory, obviously.)
 */
int
msagCounterName_get( msagCounterTable_rowreq_ctx *rowreq_ctx, char **msagCounterName_val_ptr_ptr, size_t *msagCounterName_val_ptr_len_ptr )
{
   /** we should have a non-NULL pointer and enough storage */
   netsnmp_assert( (NULL != msagCounterName_val_ptr_ptr) && (NULL != *msagCounterName_val_ptr_ptr));
   netsnmp_assert( NULL != msagCounterName_val_ptr_len_ptr );


    DEBUGMSGTL(("verbose:msagCounterTable:msagCounterName_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the msagCounterName data.
 * copy (* msagCounterName_val_ptr_ptr ) data and (* msagCounterName_val_ptr_len_ptr ) from rowreq_ctx->data
 */
    /*
     * make sure there is enough space for msagCounterName data
     */
    if ((NULL == (* msagCounterName_val_ptr_ptr )) ||
        ((* msagCounterName_val_ptr_len_ptr ) <
         (rowreq_ctx->data.msagCounterName_len* sizeof(rowreq_ctx->data.msagCounterName[0])))) {
        /*
         * allocate space for msagCounterName data
         */
        (* msagCounterName_val_ptr_ptr ) = malloc(rowreq_ctx->data.msagCounterName_len* sizeof(rowreq_ctx->data.msagCounterName[0]));
        if(NULL == (* msagCounterName_val_ptr_ptr )) {
            snmp_log(LOG_ERR,"could not allocate memory\n");
            return MFD_ERROR;
        }
    }
    (* msagCounterName_val_ptr_len_ptr ) = rowreq_ctx->data.msagCounterName_len* sizeof(rowreq_ctx->data.msagCounterName[0]);
    memcpy( (* msagCounterName_val_ptr_ptr ), rowreq_ctx->data.msagCounterName, rowreq_ctx->data.msagCounterName_len* sizeof(rowreq_ctx->data.msagCounterName[0]) );

    return MFD_SUCCESS;
} /* msagCounterName_get */

/*---------------------------------------------------------------------
 * EYELINE-MSAG-MIB::msagCounterEntry.msagCounterValue
 * msagCounterValue is subid 3 of msagCounterEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.26757.2.4.1.3
 * Description:
MSAG Counter Value.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   0
 *
 *
 * Its syntax is COUNTER64 (based on perltype COUNTER64)
 * The net-snmp type is ASN_COUNTER64. The C type decl is U64 (U64)
 */
/**
 * Extract the current value of the msagCounterValue data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param msagCounterValue_val_ptr
 *        Pointer to storage for a U64 variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
msagCounterValue_get( msagCounterTable_rowreq_ctx *rowreq_ctx, U64 * msagCounterValue_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != msagCounterValue_val_ptr );

/*
 * TODO:231:o: |-> copy msagCounterValue data.
 * get (* msagCounterValue_val_ptr ).low and (* msagCounterValue_val_ptr ).high from rowreq_ctx->data
 */
    (* msagCounterValue_val_ptr ).high = rowreq_ctx->data.msagCounterValue.high;
    (* msagCounterValue_val_ptr ).low = rowreq_ctx->data.msagCounterValue.low;


    return MFD_SUCCESS;
} /* msagCounterValue_get */



/** @} */