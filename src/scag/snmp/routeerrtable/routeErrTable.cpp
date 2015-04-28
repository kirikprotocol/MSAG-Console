/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.32.2.3 $ of : mfd-top.m2c,v $
 *
 * $Id$
 */
/** \mainpage MFD helper for routeErrTable
 *
 * \section intro Introduction
 * Introductory text.
 *
 */
/* standard Net-SNMP includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* include our parent header */
#include "routeErrTable.hpp"

#include <net-snmp/agent/mib_modules.h>

#include "routeErrTable_interface.hpp"

namespace scag2{
namespace snmp{
namespace routeerrtable{

smsc::logger::Logger* log = 0;
const char* containerName = "routeErrTableContainer";

oid routeErrTable_oid[] = { ROUTEERRTABLE_OID };
int routeErrTable_oid_size = OID_LENGTH(routeErrTable_oid);

void initialize_table_routeErrTable(void);


/**
 * Initializes the routeErrTable module
 */
void
init_routeErrTable(void)
{
    DEBUGMSGTL(("verbose:routeErrTable:init_routeErrTable","called\n"));

    /*
     * TODO:300:o: Perform routeErrTable one-time module initialization.
     */

    /*
     * here we initialize all the tables we're planning on supporting
     */
    if (should_init("routeErrTable"))
        initialize_table_routeErrTable();

} /* init_routeErrTable */

/**
 * Initialize the table routeErrTable
 *    (Define its contents and how it's structured)
 */
void
initialize_table_routeErrTable(void)
{
    routeErrTable_registration_ptr user_context;
    u_long flags;

    DEBUGMSGTL(("verbose:routeErrTable:initialize_table_routeErrTable","called\n"));

    /*
     * TODO:301:o: Perform routeErrTable one-time table initialization.
     */

    /*
     * TODO:302:o: |->Initialize routeErrTable user context
     * if you'd like to pass in a pointer to some data for this
     * table, allocate or set it up here.
     */
    /*
     * a netsnmp_data_list is a simple way to store void pointers. A simple
     * string token is used to add, find or remove pointers.
     */
    user_context = netsnmp_create_data_list("routeErrTable", NULL, NULL);

    /*
     * No support for any flags yet, but in the future you would
     * set any flags here.
     */
    flags = 0;

    /*
     * call interface initialization code
     */
    _routeErrTable_initialize_interface(user_context, flags);
} /* initialize_table_routeErrTable */

/**
 * extra context initialization
 *
 * @param rowreq_ctx    : row request context
 * @param user_init_ctx : void pointer for user (parameter to rowreq_ctx_allocate)
 *
 * @retval MFD_SUCCESS  : no errors
 * @retval MFD_ERROR    : error (context allocate will fail)
 */
int
routeErrTable_rowreq_ctx_init(routeErrTable_rowreq_ctx *rowreq_ctx)
{
    DEBUGMSGTL(("verbose:routeErrTable:routeErrTable_rowreq_ctx_init","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    /*
     * TODO:210:o: |-> Perform extra routeErrTable rowreq initialization.
     */

    return MFD_SUCCESS;
} /* routeErrTable_rowreq_ctx_init */

/**
 * extra context cleanup
 *
 */
void routeErrTable_rowreq_ctx_cleanup(routeErrTable_rowreq_ctx *rowreq_ctx)
{
    DEBUGMSGTL(("verbose:routeErrTable:routeErrTable_rowreq_ctx_cleanup","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    /*
     * TODO:211:o: |-> Perform extra routeErrTable rowreq cleanup.
     */
} /* routeErrTable_rowreq_ctx_cleanup */

/**
 * pre-request callback
 *
 *
 * @retval MFD_SUCCESS              : success.
 * @retval MFD_ERROR                : other error
 */
int
routeErrTable_pre_request(routeErrTable_registration_ptr user_context)
{
    DEBUGMSGTL(("verbose:routeErrTable:routeErrTable_pre_request","called\n"));

    /*
     * TODO:510:o: Perform routeErrTable pre-request actions.
     */

    return MFD_SUCCESS;
} /* routeErrTable_pre_request */

/**
 * post-request callback
 *
 *
 * @retval MFD_SUCCESS : success.
 * @retval MFD_ERROR   : other error (ignored)
 */
int
routeErrTable_post_request(routeErrTable_registration_ptr user_context)
{
    DEBUGMSGTL(("verbose:routeErrTable:routeErrTable_post_request","called\n"));

    /*
     * TODO:511:o: Perform routeErrTable pos-request actions.
     */

    return MFD_SUCCESS;
} /* routeErrTable_post_request */


}//routeErrtable
}//snmp
}//smsc

/** @{ */
