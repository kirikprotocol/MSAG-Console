/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.18.2.1 $ of : mfd-data-get.m2c,v $
 *
 * $Id$
 */
/* standard Net-SNMP includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* include our parent header */
#include "smeStatTable.hpp"

namespace smsc{
namespace snmp{
namespace smestattable{


/** @defgroup data_get data_get: Routines to get data
 *
 * TODO:230:M: Implement smeStatTable get routines.
 * TODO:240:M: Implement smeStatTable mapping routines (if any).
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
 *** Table smeStatTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * smeStatTable is subid 10 of smsc.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.1.10, length: 9
*/

/* ---------------------------------------------------------------------
 * TODO:200:r: Implement smeStatTable data context functions.
 */

/*---------------------------------------------------------------------
 * SIBINCO-SMSC-MIB::smeStatEntry.smeStatIndex
 * smeStatIndex is subid 1 of smeStatEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.26757.1.10.1.1
 * Description:
SME Index.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 1      hashint   0
 *   settable   0
 *
 * Ranges:  0 - 2000;
 *
 * Its syntax is INTEGER (based on perltype INTEGER)
 * The net-snmp type is ASN_INTEGER. The C type decl is long (long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-smeStatIndex.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
smeStatIndex_map(long *mib_smeStatIndex_val_ptr, long raw_smeStatIndex_val)
{
    netsnmp_assert(NULL != mib_smeStatIndex_val_ptr);

    DEBUGMSGTL(("verbose:smeStatTable:smeStatIndex_map","called\n"));

    /*
     * TODO:241:o: |-> Implement smeStatIndex mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_smeStatIndex_val_ptr) = raw_smeStatIndex_val;

    return MFD_SUCCESS;
} /* smeStatIndex_map */


/**
 * set mib index(es)
 *
 * @param tbl_idx mib index structure
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
smeStatTable_indexes_set_tbl_idx(smeStatTable_mib_index *tbl_idx, long smeStatIndex_val)
{
    DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_indexes_set_tbl_idx","called\n"));

    /* smeStatIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/R/d/h */
    tbl_idx->smeStatIndex = smeStatIndex_val;


    return MFD_SUCCESS;
} /* smeStatTable_indexes_set_tbl_idx */

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
smeStatTable_indexes_set(smeStatTable_rowreq_ctx *rowreq_ctx, long smeStatIndex_val)
{
    DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_indexes_set","called\n"));

    if(MFD_SUCCESS != smeStatTable_indexes_set_tbl_idx(&rowreq_ctx->tbl_idx
                                   , smeStatIndex_val
           ))
        return MFD_ERROR;

    /*
     * convert mib index to oid index
     */
    rowreq_ctx->oid_idx.len = (int)(sizeof(rowreq_ctx->oid_tmp) / sizeof(oid));
    if(0 != smeStatTable_index_to_oid(&rowreq_ctx->oid_idx,
                                    &rowreq_ctx->tbl_idx)) {
        return MFD_ERROR;
    }

    return MFD_SUCCESS;
} /* smeStatTable_indexes_set */


/*---------------------------------------------------------------------
 * SIBINCO-SMSC-MIB::smeStatEntry.smeStatSystemId
 * smeStatSystemId is subid 2 of smeStatEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.26757.1.10.1.2
 * Description:
SME system Id.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 1      hashint   1
 *   settable   0
 *   hint: 255a
 *
 * Ranges:  0 - 32;
 *
 * Its syntax is DisplayString (based on perltype OCTETSTR)
 * The net-snmp type is ASN_OCTET_STR. The C type decl is char (char)
 * This data type requires a length.  (Max 32)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-smeStatSystemId.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
smeStatSystemId_map(char **mib_smeStatSystemId_val_ptr_ptr, size_t *mib_smeStatSystemId_val_ptr_len_ptr, char *raw_smeStatSystemId_val_ptr, size_t raw_smeStatSystemId_val_ptr_len, int allow_realloc)
{
    size_t converted_len;

    netsnmp_assert(NULL != raw_smeStatSystemId_val_ptr);
    netsnmp_assert((NULL != mib_smeStatSystemId_val_ptr_ptr) && (NULL != mib_smeStatSystemId_val_ptr_len_ptr));

    DEBUGMSGTL(("verbose:smeStatTable:smeStatSystemId_map","called\n"));

    /*
     * TODO:241:r: |-> Implement smeStatSystemId non-integer mapping
     * it is hard to autogenerate code for mapping types that are not simple
     * integers, so here is an idea of what you might need to do. It will
     * probably need some tweaking to get right.
     */
    /*
     * if the length of the raw data doesn't directly correspond with
     * the length of the mib data, set converted_len to the
     * space required.
     */
    converted_len = raw_smeStatSystemId_val_ptr_len; /* assume equal */
    if((NULL == *mib_smeStatSystemId_val_ptr_ptr) || (*mib_smeStatSystemId_val_ptr_len_ptr < converted_len)) {
        if(! allow_realloc) {
            snmp_log(LOG_ERR,"not enough space for value mapping\n");
            return SNMP_ERR_GENERR;
        }
        *mib_smeStatSystemId_val_ptr_ptr = (char*)realloc( *mib_smeStatSystemId_val_ptr_ptr, converted_len * sizeof(**mib_smeStatSystemId_val_ptr_ptr));
        if(NULL == *mib_smeStatSystemId_val_ptr_ptr) {
            snmp_log(LOG_ERR,"could not allocate memory\n");
            return SNMP_ERR_GENERR;
        }
    }
    *mib_smeStatSystemId_val_ptr_len_ptr = converted_len;
    memcpy( *mib_smeStatSystemId_val_ptr_ptr, raw_smeStatSystemId_val_ptr, converted_len );

    return MFD_SUCCESS;
} /* smeStatSystemId_map */

/**
 * Extract the current value of the smeStatSystemId data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param smeStatSystemId_val_ptr_ptr
 *        Pointer to storage for a char variable
 * @param smeStatSystemId_val_ptr_len_ptr
 *        Pointer to a size_t. On entry, it will contain the size (in bytes)
 *        pointed to by smeStatSystemId.
 *        On exit, this value should contain the data size (in bytes).
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
*
 * @note If you need more than (*smeStatSystemId_val_ptr_len_ptr) bytes of memory,
 *       allocate it using malloc() and update smeStatSystemId_val_ptr_ptr.
 *       <b>DO NOT</b> free the previous pointer.
 *       The MFD helper will release the memory you allocate.
 *
 * @remark If you call this function yourself, you are responsible
 *         for checking if the pointer changed, and freeing any
 *         previously allocated memory. (Not necessary if you pass
 *         in a pointer to static memory, obviously.)
 */
int
smeStatSystemId_get( smeStatTable_rowreq_ctx *rowreq_ctx, char **smeStatSystemId_val_ptr_ptr, size_t *smeStatSystemId_val_ptr_len_ptr )
{
   /** we should have a non-NULL pointer and enough storage */
   netsnmp_assert( (NULL != smeStatSystemId_val_ptr_ptr) && (NULL != *smeStatSystemId_val_ptr_ptr));
   netsnmp_assert( NULL != smeStatSystemId_val_ptr_len_ptr );


    DEBUGMSGTL(("verbose:smeStatTable:smeStatSystemId_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the smeStatSystemId data.
 * set (* smeStatSystemId_val_ptr_ptr ) and (* smeStatSystemId_val_ptr_len_ptr ) from rowreq_ctx->data
 */
    /*
     * make sure there is enough space for smeStatSystemId data
     */
    if ((NULL == (* smeStatSystemId_val_ptr_ptr )) ||
        ((* smeStatSystemId_val_ptr_len_ptr ) < (rowreq_ctx->data.smeStatSystemId_len * sizeof((* smeStatSystemId_val_ptr_ptr )[0])))) {
        /*
         * allocate space for smeStatSystemId data
         */
        (* smeStatSystemId_val_ptr_ptr ) = (char*)malloc(rowreq_ctx->data.smeStatSystemId_len * sizeof((* smeStatSystemId_val_ptr_ptr )[0]));
        if(NULL == (* smeStatSystemId_val_ptr_ptr )) {
            snmp_log(LOG_ERR,"could not allocate memory\n");
            return MFD_ERROR;
        }
    }
    (* smeStatSystemId_val_ptr_len_ptr ) = rowreq_ctx->data.smeStatSystemId_len * sizeof((* smeStatSystemId_val_ptr_ptr )[0]);
    memcpy( (* smeStatSystemId_val_ptr_ptr ), rowreq_ctx->data.smeStatSystemId, (* smeStatSystemId_val_ptr_len_ptr ) );

    return MFD_SUCCESS;
} /* smeStatSystemId_get */

/*---------------------------------------------------------------------
 * SIBINCO-SMSC-MIB::smeStatEntry.smeStatAccepted
 * smeStatAccepted is subid 3 of smeStatEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.26757.1.10.1.3
 * Description:
SME Counter Accepted
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
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-smeStatAccepted.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
smeStatAccepted_map(U64 *mib_smeStatAccepted_val_ptr, U64 raw_smeStatAccepted_val)
{
    netsnmp_assert(NULL != mib_smeStatAccepted_val_ptr);

    DEBUGMSGTL(("verbose:smeStatTable:smeStatAccepted_map","called\n"));

    /*
     * TODO:241:o: |-> Implement smeStatAccepted mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_smeStatAccepted_val_ptr) = raw_smeStatAccepted_val;

    return MFD_SUCCESS;
} /* smeStatAccepted_map */

/**
 * Extract the current value of the smeStatAccepted data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param smeStatAccepted_val_ptr
 *        Pointer to storage for a U64 variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
smeStatAccepted_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatAccepted_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != smeStatAccepted_val_ptr );

/*
 * TODO:231:o: |-> copy smeStatAccepted data.
 * get (* smeStatAccepted_val_ptr ).low and (* smeStatAccepted_val_ptr ).high from rowreq_ctx->data
 */
    (* smeStatAccepted_val_ptr ).high = rowreq_ctx->data.smeStatAccepted.high;
    (* smeStatAccepted_val_ptr ).low = rowreq_ctx->data.smeStatAccepted.low;


    return MFD_SUCCESS;
} /* smeStatAccepted_get */

/*---------------------------------------------------------------------
 * SIBINCO-SMSC-MIB::smeStatEntry.smeStatRejected
 * smeStatRejected is subid 4 of smeStatEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.26757.1.10.1.4
 * Description:
SME Counter Rejected
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
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-smeStatRejected.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
smeStatRejected_map(U64 *mib_smeStatRejected_val_ptr, U64 raw_smeStatRejected_val)
{
    netsnmp_assert(NULL != mib_smeStatRejected_val_ptr);

    DEBUGMSGTL(("verbose:smeStatTable:smeStatRejected_map","called\n"));

    /*
     * TODO:241:o: |-> Implement smeStatRejected mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_smeStatRejected_val_ptr) = raw_smeStatRejected_val;

    return MFD_SUCCESS;
} /* smeStatRejected_map */

/**
 * Extract the current value of the smeStatRejected data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param smeStatRejected_val_ptr
 *        Pointer to storage for a U64 variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
smeStatRejected_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatRejected_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != smeStatRejected_val_ptr );

/*
 * TODO:231:o: |-> copy smeStatRejected data.
 * get (* smeStatRejected_val_ptr ).low and (* smeStatRejected_val_ptr ).high from rowreq_ctx->data
 */
    (* smeStatRejected_val_ptr ).high = rowreq_ctx->data.smeStatRejected.high;
    (* smeStatRejected_val_ptr ).low = rowreq_ctx->data.smeStatRejected.low;


    return MFD_SUCCESS;
} /* smeStatRejected_get */

/*---------------------------------------------------------------------
 * SIBINCO-SMSC-MIB::smeStatEntry.smeStatDelivered
 * smeStatDelivered is subid 5 of smeStatEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.26757.1.10.1.5
 * Description:
SME Counter Delivered
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
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-smeStatDelivered.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
smeStatDelivered_map(U64 *mib_smeStatDelivered_val_ptr, U64 raw_smeStatDelivered_val)
{
    netsnmp_assert(NULL != mib_smeStatDelivered_val_ptr);

    DEBUGMSGTL(("verbose:smeStatTable:smeStatDelivered_map","called\n"));

    /*
     * TODO:241:o: |-> Implement smeStatDelivered mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_smeStatDelivered_val_ptr) = raw_smeStatDelivered_val;

    return MFD_SUCCESS;
} /* smeStatDelivered_map */

/**
 * Extract the current value of the smeStatDelivered data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param smeStatDelivered_val_ptr
 *        Pointer to storage for a U64 variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
smeStatDelivered_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatDelivered_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != smeStatDelivered_val_ptr );

/*
 * TODO:231:o: |-> copy smeStatDelivered data.
 * get (* smeStatDelivered_val_ptr ).low and (* smeStatDelivered_val_ptr ).high from rowreq_ctx->data
 */
    (* smeStatDelivered_val_ptr ).high = rowreq_ctx->data.smeStatDelivered.high;
    (* smeStatDelivered_val_ptr ).low = rowreq_ctx->data.smeStatDelivered.low;


    return MFD_SUCCESS;
} /* smeStatDelivered_get */

/*---------------------------------------------------------------------
 * SIBINCO-SMSC-MIB::smeStatEntry.smeStatFailed
 * smeStatFailed is subid 6 of smeStatEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.26757.1.10.1.6
 * Description:
SME Counter Failed
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
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-smeStatFailed.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
smeStatFailed_map(U64 *mib_smeStatFailed_val_ptr, U64 raw_smeStatFailed_val)
{
    netsnmp_assert(NULL != mib_smeStatFailed_val_ptr);

    DEBUGMSGTL(("verbose:smeStatTable:smeStatFailed_map","called\n"));

    /*
     * TODO:241:o: |-> Implement smeStatFailed mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_smeStatFailed_val_ptr) = raw_smeStatFailed_val;

    return MFD_SUCCESS;
} /* smeStatFailed_map */

/**
 * Extract the current value of the smeStatFailed data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param smeStatFailed_val_ptr
 *        Pointer to storage for a U64 variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
smeStatFailed_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatFailed_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != smeStatFailed_val_ptr );

/*
 * TODO:231:o: |-> copy smeStatFailed data.
 * get (* smeStatFailed_val_ptr ).low and (* smeStatFailed_val_ptr ).high from rowreq_ctx->data
 */
    (* smeStatFailed_val_ptr ).high = rowreq_ctx->data.smeStatFailed.high;
    (* smeStatFailed_val_ptr ).low = rowreq_ctx->data.smeStatFailed.low;


    return MFD_SUCCESS;
} /* smeStatFailed_get */

/*---------------------------------------------------------------------
 * SIBINCO-SMSC-MIB::smeStatEntry.smeStatTemporal
 * smeStatTemporal is subid 7 of smeStatEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.26757.1.10.1.7
 * Description:
SME Counter Temporal Errors
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
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-smeStatTemporal.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
smeStatTemporal_map(U64 *mib_smeStatTemporal_val_ptr, U64 raw_smeStatTemporal_val)
{
    netsnmp_assert(NULL != mib_smeStatTemporal_val_ptr);

    DEBUGMSGTL(("verbose:smeStatTable:smeStatTemporal_map","called\n"));

    /*
     * TODO:241:o: |-> Implement smeStatTemporal mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_smeStatTemporal_val_ptr) = raw_smeStatTemporal_val;

    return MFD_SUCCESS;
} /* smeStatTemporal_map */

/**
 * Extract the current value of the smeStatTemporal data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param smeStatTemporal_val_ptr
 *        Pointer to storage for a U64 variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
smeStatTemporal_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatTemporal_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != smeStatTemporal_val_ptr );

/*
 * TODO:231:o: |-> copy smeStatTemporal data.
 * get (* smeStatTemporal_val_ptr ).low and (* smeStatTemporal_val_ptr ).high from rowreq_ctx->data
 */
    (* smeStatTemporal_val_ptr ).high = rowreq_ctx->data.smeStatTemporal.high;
    (* smeStatTemporal_val_ptr ).low = rowreq_ctx->data.smeStatTemporal.low;


    return MFD_SUCCESS;
} /* smeStatTemporal_get */

int
smeStatRetried_map(U64 *mib_smeStatRetried_val_ptr, U64 raw_smeStatRetried_val)
{
    netsnmp_assert(NULL != mib_smeStatRetried_val_ptr);

    DEBUGMSGTL(("verbose:smeStatTable:smeStatRetried_map","called\n"));

    /*
     * TODO:241:o: |-> Implement smeStatRetried mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_smeStatRetried_val_ptr) = raw_smeStatRetried_val;

    return MFD_SUCCESS;
} /* smeStatRetried_map */
int
smeStatRetried_get( smeStatTable_rowreq_ctx *rowreq_ctx, U64 * smeStatRetried_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != smeStatRetried_val_ptr );

/*
 * TODO:231:o: |-> copy smeStatRetried data.
 * get (* smeStatRetried_val_ptr ).low and (* smeStatRetried_val_ptr ).high from rowreq_ctx->data
 */
    (* smeStatRetried_val_ptr ).high = rowreq_ctx->data.smeStatRetried.high;
    (* smeStatRetried_val_ptr ).low = rowreq_ctx->data.smeStatRetried.low;


    return MFD_SUCCESS;
} /* smeStatRetried_get */


}//namespace smestattable{
}//namespace snmp{
}//namespace smsc{


/** @} */
