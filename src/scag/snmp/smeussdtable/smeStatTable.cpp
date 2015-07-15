/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 17337 $ of $ 
 *
 * $Id$
 */
/** \page MFD helper for smeStatTable
 *
 * \section intro Introduction
 * Introductory text.
 *
 */
/* standard Net-SNMP includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/mib_modules.h>

/* include our parent header */
#include "smeStatTable.hpp"
#include "smeStatTable_interface.hpp"

namespace scag2 {
namespace snmp {
namespace smeussdtable {

smsc::logger::Logger* log = 0;
const char* containerName = "smeUssdTableContainer";

oid smeStatTable_oid[] = { SMESTATTABLE_OID };
int smeStatTable_oid_size = OID_LENGTH(smeStatTable_oid);

smeStatTable_registration_ptr  smeStatTable_user_context;

void initialize_table_smeStatTable(void);
//void shutdown_table_smeStatTable(void);


/**
 * Initializes the smeStatTable module
 */
void init_smeStatTable(void)
{
    DEBUGMSGTL(("verbose:smeStatTable:init_smeStatTable","called\n"));

    /*
     * TODO:300:o: Perform smeStatTable one-time module initialization.
     */
     
    /*
     * here we initialize all the tables we're planning on supporting
     */
    if (should_init("smeUssdTable"))
        initialize_table_smeStatTable();

} /* init_smeStatTable */

/**
 * Shut-down the smeStatTable module (agent is exiting)
 */
/*
void shutdown_smeStatTable(void)
{
    if (should_init("smeUssdTable"))
        shutdown_table_smeStatTable();

}
*/

/**
 * Initialize the table smeStatTable 
 *    (Define its contents and how it's structured)
 */
void initialize_table_smeStatTable(void)
{
    smeStatTable_registration_ptr user_context;
    u_long flags;

    DEBUGMSGTL(("verbose:smeStatTable:initialize_table_smeStatTable","called\n"));

    /*
     * TODO:301:o: Perform smeStatTable one-time table initialization.
     */

    /*
     * TODO:302:o: |->Initialize smeStatTable user context
     * if you'd like to pass in a pointer to some data for this
     * table, allocate or set it up here.
     */
    /*
     * a netsnmp_data_list is a simple way to store void pointers. A simple
     * string token is used to add, find or remove pointers.
     */
    user_context = netsnmp_create_data_list("smeUssdTable", NULL, NULL);
    
    /*
     * No support for any flags yet, but in the future you would
     * set any flags here.
     */
    flags = 0;
    
    /*
     * call interface initialization code
     */
    _smeStatTable_initialize_interface(user_context, flags);
} /* initialize_table_smeStatTable */

/**
 * Shutdown the table smeStatTable 
 */
/*
void shutdown_table_smeStatTable(void)
{
    _smeStatTable_shutdown_interface(&smeStatTable_user_context);
}
*/

/**
 * extra context initialization (eg default values)
 *
 * @param rowreq_ctx    : row request context
 * @param user_init_ctx : void pointer for user (parameter to rowreq_ctx_allocate)
 *
 * @retval MFD_SUCCESS  : no errors
 * @retval MFD_ERROR    : error (context allocate will fail)
 *
int smeStatTable_rowreq_ctx_init(smeStatTable_rowreq_ctx* rowreq_ctx, void *user_init_ctx)
{
    DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_rowreq_ctx_init","called\n"));
    netsnmp_assert(NULL != rowreq_ctx);
    return MFD_SUCCESS;
}
*/

/**
 * extra context cleanup
 *
 *
void smeStatTable_rowreq_ctx_cleanup(smeStatTable_rowreq_ctx* rowreq_ctx)
{
    DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_rowreq_ctx_cleanup","called\n"));
    netsnmp_assert(NULL != rowreq_ctx);
}
*/

/**
 * pre-request callback
 *
 *
 * @retval MFD_SUCCESS              : success.
 * @retval MFD_ERROR                : other error
 */
int smeStatTable_pre_request(smeStatTable_registration_ptr user_context)
{
//    DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_pre_request","called\n"));

    /*
     * TODO:510:o: Perform smeStatTable pre-request actions.
     */

    return MFD_SUCCESS;
} /* smeStatTable_pre_request */

/**
 * post-request callback
 *
 * Note:
 *   New rows have been inserted into the container, and
 *   deleted rows have been removed from the container and
 *   released.
 *
 * @param user_context
 * @param rc : MFD_SUCCESS if all requests succeeded
 *
 * @retval MFD_SUCCESS : success.
 * @retval MFD_ERROR   : other error (ignored)
 */
int smeStatTable_post_request(smeStatTable_registration_ptr user_context)
{
//    DEBUGMSGTL(("verbose:smeStatTable:smeStatTable_post_request","called\n"));

    /*
     * TODO:511:o: Perform smeStatTable post-request actions.
     */

    return MFD_SUCCESS;
} /* smeStatTable_post_request */

}}}

/** @{ */