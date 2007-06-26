/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.32.2.3 $ of : mfd-top.m2c,v $
 *
 * $Id$
 */
#ifndef SMESTATTABLE_H
#define SMESTATTABLE_H


/** @defgroup misc misc: Miscelaneous routines
 *
 * @{
 */
#include <net-snmp/library/asn1.h>

namespace smsc{
namespace smeman{
class SmeManager;
}
namespace stat{
class SmeStats;
}
}

namespace smsc{
namespace snmp{
namespace smestattable{

smsc::smeman::SmeManager* getSmeMan();
smsc::stat::SmeStats* getSmeStats();


/* OID and column number definitions for  */
#include "smeStatTable_oids.hpp"


/* *********************************************************************
 * function declarations
 */
void init_smeStatTable(void);

/* *********************************************************************
 * Table declarations
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
/* *********************************************************************
 * When you register your mib, you get to provide a generic
 * pointer that will be passed back to you for most of the
 * functions calls.
 *
 * TODO:100:r: Review all context structures
 */
    /*
     * TODO:101:o: |-> Review smeStatTable registration context.
     */
typedef netsnmp_data_list * smeStatTable_registration_ptr;

/**********************************************************************/
/*
 * TODO:110:r: |-> Review smeStatTable data context structure.
 * This structure is used to represent the data for smeStatTable.
 */
/*
 * This structure contains storage for all the columns defined in the
 * smeStatTable.
 */
typedef struct smeStatTable_data_s {

        /*
         * smeStatSystemId(2)/DisplayString/ASN_OCTET_STR/char(char)//L/A/w/e/R/d/H
         */
   char   smeStatSystemId[32];
size_t      smeStatSystemId_len; /* # of char elements, not bytes */

        /*
         * smeStatAccepted(3)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   smeStatAccepted;

        /*
         * smeStatRejected(4)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   smeStatRejected;

   U64   smeStatRetried;

        /*
         * smeStatDelivered(5)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   smeStatDelivered;

        /*
         * smeStatFailed(6)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   smeStatFailed;

        /*
         * smeStatTemporal(7)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   smeStatTemporal;

} smeStatTable_data;


/*
 * TODO:120:r: |-> Review smeStatTable mib index.
 * This structure is used to represent the index for smeStatTable.
 */
typedef struct smeStatTable_mib_index_s {

        /*
         * smeStatIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/R/d/h
         */
   long   smeStatIndex;


} smeStatTable_mib_index;

    /*
     * TODO:121:r: |   |-> Review smeStatTable max index length.
     * If you KNOW that your indexes will never exceed a certain
     * length, update this macro to that length.
*/
#define MAX_smeStatTable_IDX_LEN     1


/* *********************************************************************
 * TODO:130:o: |-> Review smeStatTable Row request (rowreq) context.
 * When your functions are called, you will be passed a
 * smeStatTable_rowreq_ctx pointer.
 */
typedef struct smeStatTable_rowreq_ctx_s {

    /** this must be first for container compare to work */
    netsnmp_index        oid_idx;
    oid                  oid_tmp[MAX_smeStatTable_IDX_LEN];

    smeStatTable_mib_index        tbl_idx;

    smeStatTable_data              data;

    /*
     * flags per row. Currently, the first (lower) 8 bits are reserved
     * for the user. See mfd.h for other flags.
     */
    u_int                       rowreq_flags;

    /*
     * implementor's context pointer (provided during registration)
     */
    smeStatTable_registration_ptr smeStatTable_reg;

    /*
     * TODO:131:o: |   |-> Add useful data to smeStatTable rowreq context.
     */

    /*
     * storage for future expansion
     */
    netsnmp_data_list             *smeStatTable_data_list;

} smeStatTable_rowreq_ctx;

typedef struct smeStatTable_ref_rowreq_ctx_s {
    smeStatTable_rowreq_ctx *rowreq_ctx;
} smeStatTable_ref_rowreq_ctx;

/* *********************************************************************
 * function prototypes
 */
    int smeStatTable_pre_request(smeStatTable_registration_ptr user_context);
    int smeStatTable_post_request(smeStatTable_registration_ptr user_context);


extern oid smeStatTable_oid[];
extern int smeStatTable_oid_size;


}//namespace smestattable{
}//namespace snmp{
}//namespace smsc{

#include "smeStatTable_interface.hpp"
#include "smeStatTable_data_access.hpp"
#include "smeStatTable_data_get.hpp"


#endif /* SMESTATTABLE_H */
