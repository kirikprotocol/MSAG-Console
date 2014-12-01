/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 17337 $ of $
 *
 * $Id$
 */
#ifndef ROUTESTATTABLE_H
#define ROUTESTATTABLE_H

#include <net-snmp/library/asn1.h>

/** @addtogroup misc misc: Miscellaneous routines
 *
 * @{
 */
#include <net-snmp/library/asn1.h>

/* other required module components */
    /* *INDENT-OFF*  */

/*
config_add_mib(EYELINE-MSAG-MIB)
config_require(EYELINE-MSAG-MIB/routeStatTable/routeStatTable_interface)
config_require(EYELINE-MSAG-MIB/routeStatTable/routeStatTable_data_access)
config_require(EYELINE-MSAG-MIB/routeStatTable/routeStatTable_data_get)
config_require(EYELINE-MSAG-MIB/routeStatTable/routeStatTable_data_set)
*/

/* *INDENT-ON*  */

/* OID and column number definitions for routeStatTable */
#include "routeStatTable_oids.hpp"

/* enum definions */
//#include "routeStatTable_enums.hpp"


namespace scag2 {
namespace snmp {
namespace routestattable {


/* *********************************************************************
 * function declarations
 */
void init_routeStatTable(void);
void shutdown_routeStatTable(void);

/* *********************************************************************
 * Table declarations
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
/* *********************************************************************
 * When you register your mib, you get to provide a generic
 * pointer that will be passed back to you for most of the
 * functions calls.
 *
 * TODO:100:r: Review all context structures
 */
    /*
     * TODO:101:o: |-> Review routeStatTable registration context.
     */
typedef netsnmp_data_list* routeStatTable_registration_ptr;

/**********************************************************************/
/*
 * TODO:110:r: |-> Review routeStatTable data context structure.
 * This structure is used to represent the data for routeStatTable.
 */
/*
 * This structure contains storage for all the columns defined in the
 * routeStatTable.
 */
typedef struct routeStatTable_data_s {
    
        /*
         * routeStatRouteId(2)/DisplayString/ASN_OCTET_STR/char(char)//L/A/w/e/R/d/H
         */
   char   routeStatRouteId[32];
size_t      routeStatRouteId_len; /* # of char elements, not bytes */
    
        /*
         * routeStatAccepted(3)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   routeStatAccepted;
    
        /*
         * routeStatRejected(4)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   routeStatRejected;
    
        /*
         * routeStatDelivered(5)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   routeStatDelivered;
    
        /*
         * routeStatGwRejected(6)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   routeStatGwRejected;
    
        /*
         * routeStatFailed(7)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   routeStatFailed;
    
        /*
         * routeStatReceiptOk(8)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   routeStatReceiptOk;
    
        /*
         * routeStatReceiptFailed(9)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   routeStatReceiptFailed;
    
} routeStatTable_data;


/*
 * TODO:120:r: |-> Review routeStatTable mib index.
 * This structure is used to represent the index for routeStatTable.
 */
typedef struct routeStatTable_mib_index_s {

        /*
         * routeStatIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/R/d/h
         */
   long   routeStatIndex;


} routeStatTable_mib_index;

    /*
     * TODO:121:r: |   |-> Review routeStatTable max index length.
     * If you KNOW that your indexes will never exceed a certain
     * length, update this macro to that length.
*/
#define MAX_routeStatTable_IDX_LEN     1


/* *********************************************************************
 * TODO:130:o: |-> Review routeStatTable Row request (rowreq) context.
 * When your functions are called, you will be passed a
 * routeStatTable_rowreq_ctx pointer.
 */
typedef struct routeStatTable_rowreq_ctx_s {

    /** this must be first for container compare to work */
    netsnmp_index        oid_idx;
    oid                  oid_tmp[MAX_routeStatTable_IDX_LEN];
    
    routeStatTable_mib_index        tbl_idx;
    
    routeStatTable_data              data;

    /*
     * flags per row. Currently, the first (lower) 8 bits are reserved
     * for the user. See mfd.h for other flags.
     */
    u_int                       rowreq_flags;

    /*
     * implementor's context pointer (provided during registration)
     */
    routeStatTable_registration_ptr routeStatTable_reg;

    /*
     * TODO:131:o: |   |-> Add useful data to routeStatTable rowreq context.
     */
    
    /*
     * storage for future expansion
     */
    netsnmp_data_list             *routeStatTable_data_list;

} routeStatTable_rowreq_ctx;

typedef struct routeStatTable_ref_rowreq_ctx_s {
    routeStatTable_rowreq_ctx *rowreq_ctx;
} routeStatTable_ref_rowreq_ctx;

/* *********************************************************************
 * function prototypes
 */
    int routeStatTable_pre_request(routeStatTable_registration_ptr user_context);
    int routeStatTable_post_request(routeStatTable_registration_ptr user_context);

//    int routeStatTable_rowreq_ctx_init(routeStatTable_rowreq_ctx *rowreq_ctx, void *user_init_ctx);
//    void routeStatTable_rowreq_ctx_cleanup(routeStatTable_rowreq_ctx *rowreq_ctx);


    routeStatTable_rowreq_ctx* routeStatTable_row_find_by_mib_index(routeStatTable_mib_index *mib_idx);

extern /*const*/ oid routeStatTable_oid[];
extern /*const*/ int routeStatTable_oid_size;

}}}

#include "routeStatTable_interface.hpp"
#include "routeStatTable_data_access.hpp"
#include "routeStatTable_data_get.hpp"

#endif /* SMESSTATTABLE_H */
/** @} */
