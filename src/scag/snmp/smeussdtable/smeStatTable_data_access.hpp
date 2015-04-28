/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 14170 $ of $
 *
 * $Id$
 */
#ifndef SMESTATTABLE_DATA_ACCESS_H
#define SMESTATTABLE_DATA_ACCESS_H


//using namespace scag2::stat;

namespace scag2 {
namespace snmp {
namespace smeussdtable {


/* *********************************************************************
 * function declarations
 */

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
 * EYELINE-MSAG-MIB::smeStatTable is subid 10 of msag.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.2.10, length: 9
*/


    int smeStatTable_init_data(smeStatTable_registration_ptr smeStatTable_reg);


/*
 * TODO:180:o: Review smeStatTable cache timeout.
 * The number of seconds before the cache times out
 */
//#define SMESTATTABLE_CACHE_TIMEOUT   60

void smeStatTable_container_init(netsnmp_container* *container_ptr_ptr, netsnmp_cache* cache);
void smeStatTable_container_shutdown(netsnmp_container* container_ptr);

int smeStatTable_cache_load(netsnmp_container* container);
void smeStatTable_cache_free(netsnmp_container* container);

    /*
    ***************************************************
    ***             START EXAMPLE CODE              ***
    ***---------------------------------------------***/
/* *********************************************************************
 * Since we have no idea how you really access your data, we'll go with
 * a worst case example: a flat text file.
 */
#define MAX_LINE_SIZE 256
    /*
    ***---------------------------------------------***
    ***              END  EXAMPLE CODE              ***
    ***************************************************/
    int smeStatTable_row_prep( smeStatTable_rowreq_ctx *rowreq_ctx);

}
}
}

#endif /* SMESTATTABLE_DATA_ACCESS_H */
