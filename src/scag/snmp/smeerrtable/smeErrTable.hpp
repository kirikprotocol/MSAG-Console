/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.32.2.3 $ of : mfd-top.m2c,v $
 *
 * $Id$
 */
#ifndef SMEERRTABLE_H
#define SMEERRTABLE_H


/** @defgroup misc misc: Miscelaneous routines
 *
 * @{
 */
#include <net-snmp/library/asn1.h>

/* OID and column number definitions for  */
#include "smeErrTable_oids.hpp"


namespace scag2{
namespace snmp{
namespace smeerrtable{

/* *********************************************************************
 * function declarations
 */
void init_smeErrTable(void);

/* *********************************************************************
 * Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table smeErrTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * smeErrTable is subid 11 of msag.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.2.11, length: 9
*/
/* *********************************************************************
 * When you register your mib, you get to provide a generic
 * pointer that will be passed back to you for most of the
 * functions calls.
 *
 * TODO:100:r: Review all context structures
 */
    /*
     * TODO:101:o: |-> Review smeErrTable registration context.
     */
typedef netsnmp_data_list * smeErrTable_registration_ptr;

/**********************************************************************/
/*
 * TODO:110:r: |-> Review smeErrTable data context structure.
 * This structure is used to represent the data for smeErrTable.
 */
/*
 * This structure contains storage for all the columns defined in the
 * smeErrTable.
 */
typedef struct smeErrTable_data_s {

        /*
         * smeErrSystemId(3)/DisplayString/ASN_OCTET_STR/char(char)//L/A/w/e/R/d/H
         */
   char   smeErrSystemId[32];
size_t      smeErrSystemId_len; /* # of char elements, not bytes */

        /*
         * smeErrCount(4)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h
         */
   U64   smeErrCount;

} smeErrTable_data;


/*
 * TODO:120:r: |-> Review smeErrTable mib index.
 * This structure is used to represent the index for smeErrTable.
 */
typedef struct smeErrTable_mib_index_s {

        /*
         * smeErrIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/R/d/h
         */
   long   smeErrIndex;

        /*
         * smeErrCode(2)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/R/d/h
         */
   long   smeErrCode;


} smeErrTable_mib_index;

    /*
     * TODO:121:r: |   |-> Review smeErrTable max index length.
     * If you KNOW that your indexes will never exceed a certain
     * length, update this macro to that length.
*/
#define MAX_smeErrTable_IDX_LEN     2


/* *********************************************************************
 * TODO:130:o: |-> Review smeErrTable Row request (rowreq) context.
 * When your functions are called, you will be passed a
 * smeErrTable_rowreq_ctx pointer.
 */
typedef struct smeErrTable_rowreq_ctx_s {

    /** this must be first for container compare to work */
    netsnmp_index        oid_idx;
    oid                  oid_tmp[MAX_smeErrTable_IDX_LEN];

    smeErrTable_mib_index        tbl_idx;

    smeErrTable_data              data;

    /*
     * flags per row. Currently, the first (lower) 8 bits are reserved
     * for the user. See mfd.h for other flags.
     */
    u_int                       rowreq_flags;

    /*
     * implementor's context pointer (provided during registration)
     */
    smeErrTable_registration_ptr smeErrTable_reg;

    /*
     * TODO:131:o: |   |-> Add useful data to smeErrTable rowreq context.
     */

    /*
     * storage for future expansion
     */
    netsnmp_data_list             *smeErrTable_data_list;

} smeErrTable_rowreq_ctx;

typedef struct smeErrTable_ref_rowreq_ctx_s {
    smeErrTable_rowreq_ctx *rowreq_ctx;
} smeErrTable_ref_rowreq_ctx;

/* *********************************************************************
 * function prototypes
 */
    int smeErrTable_pre_request(smeErrTable_registration_ptr user_context);
    int smeErrTable_post_request(smeErrTable_registration_ptr user_context);

    int smeErrTable_rowreq_ctx_init(smeErrTable_rowreq_ctx *rowreq_ctx);
    void smeErrTable_rowreq_ctx_cleanup(smeErrTable_rowreq_ctx *rowreq_ctx);


extern oid smeErrTable_oid[];
extern int smeErrTable_oid_size;

}//smeerrtable
}//snmp
}//smsc

#include "smeErrTable_interface.hpp"
#include "smeErrTable_data_access.hpp"
#include "smeErrTable_data_get.hpp"


#endif /* SMEERRTABLE_H */
