/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 18978 $ of $ 
 *
 * $Id$
 */
/*
 * *********************************************************************
 * *********************************************************************
 * *********************************************************************
 * ***                                                               ***
 * ***  NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE  ***
 * ***                                                               ***
 * ***                                                               ***
 * ***       THIS FILE DOES NOT CONTAIN ANY USER EDITABLE CODE.      ***
 * ***                                                               ***
 * ***                                                               ***
 * ***       THE GENERATED CODE IS INTERNAL IMPLEMENTATION, AND      ***
 * ***                                                               ***
 * ***                                                               ***
 * ***    IS SUBJECT TO CHANGE WITHOUT WARNING IN FUTURE RELEASES.   ***
 * ***                                                               ***
 * ***                                                               ***
 * *********************************************************************
 * *********************************************************************
 * *********************************************************************
 */

/* standard Net-SNMP includes */
#define NETSNMP_USE_INLINE
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/table_container.h>
#include <net-snmp/agent/row_merge.h>

/* include our parent header */
#include "routeStatTable.hpp"


#include <net-snmp/library/container.h>

#include "routeStatTable_interface.hpp"
#include "routeStatTable_data_access.hpp"
#include "logger/Logger.h"

/**********************************************************************
 **********************************************************************
 ***
 *** Table routeStatTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * routeStatTable is subid 12 of msag.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.26757.1.12, length: 9
*/

namespace scag2 {
namespace snmp {
namespace routeussdtable {

typedef struct routeStatTable_interface_ctx_s {

   netsnmp_container              *container;
   netsnmp_cache                  *cache; /* optional cache */

   routeStatTable_registration_ptr   user_ctx;

   netsnmp_table_registration_info tbl_info;

   netsnmp_baby_steps_access_methods access_multiplexer;

} routeStatTable_interface_ctx;

static routeStatTable_interface_ctx routeStatTable_if_ctx;

static void _routeStatTable_container_init(routeStatTable_interface_ctx* if_ctx);


static Netsnmp_Node_Handler _mfd_routeStatTable_pre_request;
static Netsnmp_Node_Handler _mfd_routeStatTable_post_request;
static Netsnmp_Node_Handler _mfd_routeStatTable_object_lookup;
static Netsnmp_Node_Handler _mfd_routeStatTable_get_values;
/**
 * @internal
 * Initialize the table routeStatTable
 *    (Define its contents and how it's structured)
 */
void _routeStatTable_initialize_interface(routeStatTable_registration_ptr reg_ptr,  u_long flags)
{
  log = smsc::logger::Logger::getInstance("snmp.sstat");
  DEBUGMSGTL(("internal:routeStatTable:_routeStatTable_initialize_interface","called\n"));

  netsnmp_baby_steps_access_methods *access_multiplexer = &routeStatTable_if_ctx.access_multiplexer;
  netsnmp_table_registration_info *tbl_info = &routeStatTable_if_ctx.tbl_info;
  netsnmp_handler_registration *reginfo;
  netsnmp_mib_handler *handler;
  int    mfd_modes = 0;



  /*************************************************
   *
   * save interface context for routeStatTable
   */
  /*
   * Setting up the table's definition
   */
  netsnmp_table_helper_add_indexes(tbl_info, ASN_INTEGER, /* index: routeStatIndex */ 0);

  /*  Define the minimum and maximum accessible columns.  This
      optimizes retrieval. */
  tbl_info->min_column = ROUTEUSSDTABLE_MIN_COL;
  tbl_info->max_column = ROUTEUSSDTABLE_MAX_COL;

  /*
   * save users context
   */
  routeStatTable_if_ctx.user_ctx = reg_ptr;

  /*
   * call data access initialization code
   */
  routeStatTable_init_data(reg_ptr);

  /*
   * set up the container
   */
  _routeStatTable_container_init(&routeStatTable_if_ctx);
  if (NULL == routeStatTable_if_ctx.container) {
    smsc_log_error(log, "could not initialize container for routeStatTable");
    snmp_log(LOG_ERR,"could not initialize container for routeStatTable\n");
    return;
  }

  /*
   * access_multiplexer: REQUIRED wrapper for get request handling
   */
  access_multiplexer->object_lookup = _mfd_routeStatTable_object_lookup;
  access_multiplexer->get_values = _mfd_routeStatTable_get_values;

  /*
   * no wrappers yet
   */
  access_multiplexer->pre_request = _mfd_routeStatTable_pre_request;
  access_multiplexer->post_request = _mfd_routeStatTable_post_request;


  /*************************************************
   *
   * Create a registration, save our reg data, register table.
   */
  DEBUGMSGTL(("routeStatTable:init_routeStatTable", "Registering routeUssdTable as a mibs-for-dummies table.\n"));
  handler = netsnmp_baby_steps_access_multiplexer_get(access_multiplexer);
  reginfo = netsnmp_handler_registration_create("routeUssdTable", handler,
                                                routeStatTable_oid,
                                                routeStatTable_oid_size,
                                                HANDLER_CAN_BABY_STEP | HANDLER_CAN_RONLY
                                                );
  if(NULL == reginfo) {
    snmp_log(LOG_ERR,"error registering table routeStatTable\n");
    smsc_log_error(log, "error registering table routeStatTable");
    return;
  }
  snmp_log(LOG_INFO,"register table routeUssdTable, success\n");
  smsc_log_debug(log, "register table routeUssdTable, success");

  reginfo->my_reg_void = &routeStatTable_if_ctx;

  /*************************************************
   *
   * set up baby steps handler, create it and inject it
   */
  if( access_multiplexer->object_lookup )           mfd_modes |= BABY_STEP_OBJECT_LOOKUP;
  if( access_multiplexer->set_values )              mfd_modes |= BABY_STEP_SET_VALUES;
  if( access_multiplexer->irreversible_commit )     mfd_modes |= BABY_STEP_IRREVERSIBLE_COMMIT;
  if( access_multiplexer->object_syntax_checks )    mfd_modes |= BABY_STEP_CHECK_OBJECT;
  if( access_multiplexer->pre_request )             mfd_modes |= BABY_STEP_PRE_REQUEST;
  if( access_multiplexer->post_request )            mfd_modes |= BABY_STEP_POST_REQUEST;
  if( access_multiplexer->undo_setup )              mfd_modes |= BABY_STEP_UNDO_SETUP;
  if( access_multiplexer->undo_cleanup )            mfd_modes |= BABY_STEP_UNDO_CLEANUP;
  if( access_multiplexer->undo_sets )               mfd_modes |= BABY_STEP_UNDO_SETS;
  if( access_multiplexer->row_creation )            mfd_modes |= BABY_STEP_ROW_CREATE;
  if( access_multiplexer->consistency_checks )      mfd_modes |= BABY_STEP_CHECK_CONSISTENCY;
  if( access_multiplexer->commit )                  mfd_modes |= BABY_STEP_COMMIT;
  if( access_multiplexer->undo_commit )             mfd_modes |= BABY_STEP_UNDO_COMMIT;

  handler = netsnmp_baby_steps_handler_get(mfd_modes);
  netsnmp_inject_handler(reginfo, handler);

  /*************************************************
   *
   * inject row_merge helper with prefix rootoid_len + 2 (entry.col)
   */
  handler = netsnmp_get_row_merge_handler((int)reginfo->rootoid_len + 2);
  netsnmp_inject_handler(reginfo, handler);

  /*************************************************
   *
   * inject container_table helper
   */
  handler = netsnmp_container_table_handler_get(tbl_info,
                                          routeStatTable_if_ctx.container,
                                          TABLE_CONTAINER_KEY_NETSNMP_INDEX);
  netsnmp_inject_handler( reginfo, handler );

  /*************************************************
   *
   * inject cache helper
   */
  if(NULL != routeStatTable_if_ctx.cache) {
    handler = netsnmp_cache_handler_get(routeStatTable_if_ctx.cache);
    netsnmp_inject_handler( reginfo, handler );
  }

  /*
   * register table
   */
  netsnmp_register_table(reginfo, tbl_info);
} /* _routeStatTable_initialize_interface */

void routeStatTable_valid_columns_set(netsnmp_column_info *vc)
{
    routeStatTable_if_ctx.tbl_info.valid_columns = vc;
} /* routeStatTable_valid_columns_set */

/**
 * @internal
 * convert the index component stored in the context to an oid
 */
int routeStatTable_index_to_oid(netsnmp_index *oid_idx, routeStatTable_mib_index *mib_idx)
{
    int err = SNMP_ERR_NOERROR;

    /*
     * temp storage for parsing indexes
     */
    /*
     * routeStatIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/R/d/h
     */
    netsnmp_variable_list var_routeStatIndex;

    /*
     * set up varbinds
     */
    memset( &var_routeStatIndex, 0x00, sizeof(var_routeStatIndex) );
    var_routeStatIndex.type = ASN_INTEGER;

    /*
     * chain temp index varbinds together
     */
    var_routeStatIndex.next_variable =  NULL;


    DEBUGMSGTL(("verbose:routeStatTable:routeStatTable_index_to_oid","called\n"));

        /* routeStatIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/R/d/h */
    snmp_set_var_value(&var_routeStatIndex, (u_char*)&mib_idx->routeStatIndex,
                       sizeof(mib_idx->routeStatIndex));


    unsigned long oidOutLen;
    err = build_oid_noalloc(oid_idx->oids, oid_idx->len, (unsigned long*)&oidOutLen,NULL, 0, &var_routeStatIndex);
    oid_idx->len=(int)oidOutLen;

    if(err)
        snmp_log(LOG_ERR,"error %d converting index to oid\n", err);

    /*
     * parsing may have allocated memory. free it.
     */
    snmp_reset_var_buffers( &var_routeStatIndex );

    return err;
} /* routeStatTable_index_to_oid */

/**
 * extract routeStatTable indexes from a netsnmp_index
 *
 * @retval SNMP_ERR_NOERROR  : no error
 * @retval SNMP_ERR_GENERR   : error
 */
int
routeStatTable_index_from_oid(netsnmp_index *oid_idx,  routeStatTable_mib_index *mib_idx)
{
    int err = SNMP_ERR_NOERROR;

    /*
     * temp storage for parsing indexes
     */
    /*
     * routeStatIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/R/d/h
     */
    netsnmp_variable_list var_routeStatIndex;

    /*
     * set up varbinds
     */
    memset( &var_routeStatIndex, 0x00, sizeof(var_routeStatIndex) );
    var_routeStatIndex.type = ASN_INTEGER;

    /*
     * chain temp index varbinds together
     */
    var_routeStatIndex.next_variable =  NULL;


    DEBUGMSGTL(("verbose:routeStatTable:routeStatTable_index_from_oid","called\n"));

    /*
     * parse the oid into the individual index components
     */
    err = parse_oid_indexes( oid_idx->oids, oid_idx->len,
                             &var_routeStatIndex );
    if (err == SNMP_ERR_NOERROR) {
        /*
         * copy out values
         */
      mib_idx->routeStatIndex = *((long *)var_routeStatIndex.val.string);
    }

    /*
     * parsing may have allocated memory. free it.
     */
    snmp_reset_var_buffers( &var_routeStatIndex );

    return err;
} /* routeStatTable_index_from_oid */


/* *********************************************************************
 * @internal
 * allocate resources for a routeStatTable_rowreq_ctx
 */
routeStatTable_rowreq_ctx* routeStatTable_allocate_rowreq_ctx(void)
{
    routeStatTable_rowreq_ctx* rowreq_ctx = SNMP_MALLOC_TYPEDEF(routeStatTable_rowreq_ctx);

    DEBUGMSGTL(("internal:routeStatTable:routeStatTable_allocate_rowreq_ctx","called\n"));

    if(NULL == rowreq_ctx) {
      smsc_log_error(log, "Couldn't allocate memory for a routeStatTable_rowreq_ctx");
      snmp_log(LOG_ERR,"Couldn't allocate memory for a ""routeStatTable_rowreq_ctx.\n");
    }

    rowreq_ctx->oid_idx.oids = rowreq_ctx->oid_tmp;

    rowreq_ctx->routeStatTable_data_list = NULL;
    rowreq_ctx->routeStatTable_reg = routeStatTable_if_ctx.user_ctx;

    return rowreq_ctx;
} /* routeStatTable_allocate_rowreq_ctx */

/*
 * @internal
 * release resources for a routeStatTable_rowreq_ctx
 */
void
routeStatTable_release_rowreq_ctx(routeStatTable_rowreq_ctx *rowreq_ctx)
{
    DEBUGMSGTL(("internal:routeStatTable:routeStatTable_release_rowreq_ctx","called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    /*
     * free index oid pointer
     */
    if(rowreq_ctx->oid_idx.oids != rowreq_ctx->oid_tmp)
        free(rowreq_ctx->oid_idx.oids);

    SNMP_FREE(rowreq_ctx);
} /* routeStatTable_release_rowreq_ctx */

/**
 * @internal
 * wrapper
 */
static int _mfd_routeStatTable_pre_request(netsnmp_mib_handler *handler,
                            netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *agtreq_info,
                            netsnmp_request_info *requests)
{
/*
    if (1 != netsnmp_row_merge_status_first(reginfo, agtreq_info)) {
        DEBUGMSGTL(("internal:routeStatTable","skipping additional pre_request\n"));
        return SNMP_ERR_NOERROR;
    }
    int rc = routeStatTable_pre_request(routeStatTable_if_ctx.user_ctx);
    if (MFD_SUCCESS != rc) {
         //nothing we can do about it but log it
        DEBUGMSGTL(("internal:routeStatTable","error %d from routeStatTable_pre_request\n", rc));
        netsnmp_request_set_error_all(requests, SNMP_VALIDATE_ERR(rc));
    }
*/
    return SNMP_ERR_NOERROR;
} /* _mfd_routeStatTable_pre_request */

/**
 * @internal
 * wrapper
 */
static int _mfd_routeStatTable_post_request(netsnmp_mib_handler *handler,
                             netsnmp_handler_registration *reginfo,
                             netsnmp_agent_request_info *agtreq_info,
                             netsnmp_request_info *requests)
{
  routeStatTable_rowreq_ctx* rowreq_ctx;

  int rc = routeStatTable_post_request(routeStatTable_if_ctx.user_ctx);
  if (MFD_SUCCESS != rc) {
      /*
       * nothing we can do about it but log it
       */
    smsc_log_error(log, "routeStatTable error %d from routeStatTable_post_request", rc);
    DEBUGMSGTL(("internal:routeStatTable","error %d from routeStatTable_post_request\n", rc));
  }

  /*
   * if there are no errors, check for and handle row creation/deletion
   */
  rc = netsnmp_check_requests_error(requests);
  if ( (SNMP_ERR_NOERROR == rc) &&
       (NULL != (rowreq_ctx = (routeStatTable_rowreq_ctx_s*)netsnmp_container_table_row_extract(requests))) )
  {
    if (rowreq_ctx->rowreq_flags & MFD_ROW_CREATED) {
      rowreq_ctx->rowreq_flags &= ~MFD_ROW_CREATED;
      CONTAINER_INSERT(routeStatTable_if_ctx.container, rowreq_ctx);
    }
    else if (rowreq_ctx->rowreq_flags & MFD_ROW_DELETED) {
      CONTAINER_REMOVE(routeStatTable_if_ctx.container, rowreq_ctx);
      routeStatTable_release_rowreq_ctx(rowreq_ctx);
    }
  }
  return SNMP_ERR_NOERROR;
} /* _mfd_routeStatTable_post_request */

/**
 * @internal
 * wrapper
 */
static int _mfd_routeStatTable_object_lookup(netsnmp_mib_handler *handler,
                         netsnmp_handler_registration *reginfo,
                         netsnmp_agent_request_info *agtreq_info,
                         netsnmp_request_info *requests)
{
    routeStatTable_rowreq_ctx *rowreq_ctx =
    	(routeStatTable_rowreq_ctx*)netsnmp_container_table_row_extract(requests);

    DEBUGMSGTL(("internal:routeStatTable:_mfd_routeStatTable_object_lookup","called\n"));

    /*
     * get our context from mfd
     * routeStatTable_interface_ctx *if_ctx =
     *             (routeStatTable_interface_ctx *)reginfo->my_reg_void;
     */

    if(NULL == rowreq_ctx) {
        netsnmp_request_set_error_all(requests, SNMP_ERR_NOCREATION);
    }
    else {
        routeStatTable_row_prep(rowreq_ctx);
    }

    return SNMP_ERR_NOERROR;
} /* _mfd_routeStatTable_object_lookup */

/***********************************************************************
 *
 * GET processing
 *
 ***********************************************************************/
/*
 * @internal
 * Retrieve the value for a particular column
 */
NETSNMP_STATIC_INLINE int _routeStatTable_get_column( routeStatTable_rowreq_ctx *rowreq_ctx,
  netsnmp_variable_list *var, int column )
{
    int rc = SNMPERR_SUCCESS;

    DEBUGMSGTL(("internal:routeStatTable:_mfd_routeStatTable_get_column","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    switch(column) {

    /* (INDEX) routeStatIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/A/w/e/R/d/h */
    case COLUMN_ROUTESTATINDEX:
    var->type = ASN_INTEGER;
    var->val_len = sizeof(long);
    (*var->val.integer) = rowreq_ctx->tbl_idx.routeStatIndex;
    break;

    /* routeStatRouteId(2)/DisplayString/ASN_OCTET_STR/char(char)//L/A/w/e/R/d/H */
    case COLUMN_routeStatRouteId:
    var->type = ASN_OCTET_STR;
    rc = routeStatRouteId_get(rowreq_ctx, (char **)&var->val.string, &var->val_len );
    break;

    /* routeStatAccepted(3)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h */
    case COLUMN_ROUTESTATACCEPTED:
    var->val_len = sizeof(U64);
    var->type = ASN_COUNTER64;
    rc = routeStatAccepted_get(rowreq_ctx, (U64 *)var->val.string );
    break;

    /* routeStatRejected(4)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h */
    case COLUMN_ROUTESTATREJECTED:
    var->val_len = sizeof(U64);
    var->type = ASN_COUNTER64;
    rc = routeStatRejected_get(rowreq_ctx, (U64 *)var->val.string );
    break;

    /* routeStatDelivered(5)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h */
    case COLUMN_ROUTESTATDELIVERED:
    var->val_len = sizeof(U64);
    var->type = ASN_COUNTER64;
    rc = routeStatDelivered_get(rowreq_ctx, (U64 *)var->val.string );
    break;

    /* routeStatGwRejected(6)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h */
    case COLUMN_ROUTESTATGWREJECTED:
    var->val_len = sizeof(U64);
    var->type = ASN_COUNTER64;
    rc = routeStatGwRejected_get(rowreq_ctx, (U64 *)var->val.string );
    break;

    /* routeStatFailed(7)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h */
    case COLUMN_ROUTESTATFAILED:
    var->val_len = sizeof(U64);
    var->type = ASN_COUNTER64;
    rc = routeStatFailed_get(rowreq_ctx, (U64 *)var->val.string );
    break;

    /* routeStatReceiptOk(8)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h */
    case COLUMN_ROUTESTATRECEIPTOK:
    var->val_len = sizeof(U64);
    var->type = ASN_COUNTER64;
    rc = routeStatReceiptOk_get(rowreq_ctx, (U64 *)var->val.string );

    break;

    /* routeStatReceiptFailed(9)/COUNTER64/ASN_COUNTER64/U64(U64)//l/A/w/e/r/d/h */
    case COLUMN_ROUTESTATRECEIPTFAILED:
    var->val_len = sizeof(U64);
    var->type = ASN_COUNTER64;
    rc = routeStatReceiptFailed_get(rowreq_ctx, (U64 *)var->val.string );
    break;

    default:
      smsc_log_error(log, "unknown column %d in _routeStatTable_get_column", column);
      snmp_log(LOG_ERR,"unknown column %d in _routeStatTable_get_column\n", column);
      break;
    }

    return rc;
} /* _routeStatTable_get_column */

int _mfd_routeStatTable_get_values(netsnmp_mib_handler *handler,
                         netsnmp_handler_registration *reginfo,
                         netsnmp_agent_request_info *agtreq_info,
                         netsnmp_request_info *requests)
{
    routeStatTable_rowreq_ctx *rowreq_ctx = (routeStatTable_rowreq_ctx *)
                  netsnmp_container_table_row_extract(requests);
    netsnmp_table_request_info * tri;
    u_char                     * old_string;
    void                      (*dataFreeHook)(void *);
    int                        rc;

    DEBUGMSGTL(("internal:routeStatTable:_mfd_routeStatTable_get_values","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    for(;requests; requests = requests->next) {
        /*
         * save old pointer, so we can free it if replaced
         */
        old_string = requests->requestvb->val.string;
        dataFreeHook = requests->requestvb->dataFreeHook;
        if(NULL == requests->requestvb->val.string) {
            requests->requestvb->val.string = requests->requestvb->buf;
            requests->requestvb->val_len = sizeof(requests->requestvb->buf);
        }
        else if(requests->requestvb->buf == requests->requestvb->val.string) {
            if(requests->requestvb->val_len != sizeof(requests->requestvb->buf))
                requests->requestvb->val_len = sizeof(requests->requestvb->buf);
        }

        /*
         * get column data
         */
        tri = netsnmp_extract_table_info(requests);
        if(NULL == tri)
            continue;

        rc = _routeStatTable_get_column(rowreq_ctx, requests->requestvb, tri->colnum);
        if(rc) {
            if(MFD_SKIP == rc) {
                requests->requestvb->type = ASN_PRIV_RETRY;
                rc = SNMP_ERR_NOERROR;
            }
        }
        else if (NULL == requests->requestvb->val.string) {
            snmp_log(LOG_ERR,"NULL varbind data pointer!\n");
            rc = SNMP_ERR_GENERR;
        }
        if(rc)
            netsnmp_request_set_error(requests, SNMP_VALIDATE_ERR(rc));

        /*
         * if the buffer wasn't used previously for the old data (i.e. it
         * was allcoated memory)  and the get routine replaced the pointer,
         * we need to free the previous pointer.
         */
        if(old_string && (old_string != requests->requestvb->buf) &&
           (requests->requestvb->val.string != old_string)) {
            if(dataFreeHook)
                (*dataFreeHook)(old_string);
            else
                free(old_string);
        }
    } /* for results */

    return SNMP_ERR_NOERROR;
} /* _mfd_routeStatTable_get_values */

/***********************************************************************
 *
 * SET processing
 *
 ***********************************************************************/

/*
 * NOT APPLICABLE (per MIB or user setting)
 */
/***********************************************************************
 *
 * DATA ACCESS
 *
 ***********************************************************************/
/**
 * @internal
 */
static int _cache_load(netsnmp_cache *cache, void *vmagic)
{
    DEBUGMSGTL(("internal:routeStatTable:_cache_load","called\n"));

    if((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR, "invalid cache for routeStatTable_cache_load\n");
        return -1;
    }

    /** should only be called for an invalid or expired cache */
    netsnmp_assert((0 == cache->valid) || (1 == cache->expired));

    /*
     * call user code
     */
    return routeStatTable_cache_load((netsnmp_container*)cache->magic);
} /* _cache_load */

/**
 * @internal
 */
static void _cache_item_free(routeStatTable_rowreq_ctx *rowreq_ctx, void *context)
{
    DEBUGMSGTL(("internal:routeStatTable:_cache_item_free","called\n"));

    if(NULL == rowreq_ctx)
        return;

    routeStatTable_release_rowreq_ctx(rowreq_ctx);
} /* _cache_item_free */

/**
 * @internal
 */
static void _cache_free(netsnmp_cache *cache, void *magic)
{
    netsnmp_container* container;

    DEBUGMSGTL(("internal:routeStatTable:_cache_free","called\n"));

    if((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR, "invalid cache in routeStatTable_cache_free\n");
        return;
    }

    container = (netsnmp_container*)cache->magic;

    /*
     * call user code
     */
    routeStatTable_cache_free(container);

    /*
     * free all items. inefficient, but easy.
     */
    CONTAINER_CLEAR(container, (netsnmp_container_obj_func *)_cache_item_free, NULL);
} /* _cache_free */

/**
 * @internal
 * initialize the iterator container with functions or wrappers
 */
void
_routeStatTable_container_init(routeStatTable_interface_ctx *if_ctx)
{
    DEBUGMSGTL(("internal:routeStatTable:_routeStatTable_container_init","called\n"));

    /*
     * set up the cache
     */
    if_ctx->cache = netsnmp_cache_create(30, /* timeout in seconds */
                                         _cache_load, _cache_free,
                                         routeStatTable_oid,
                                         routeStatTable_oid_size);

    if(NULL == if_ctx->cache) {
        snmp_log(LOG_ERR, "error creating cache for routeStatTable\n");
        return;
    }

    if_ctx->cache->flags = NETSNMP_CACHE_DONT_INVALIDATE_ON_SET;

    routeStatTable_container_init(&if_ctx->container, if_ctx->cache);
    if(NULL == if_ctx->container)
        if_ctx->container = netsnmp_container_find("routeStatTable:table_container");
    if(NULL == if_ctx->container) {
        snmp_log(LOG_ERR,"error creating container in ""routeStatTable_container_init\n");
        return;
    }
    if_ctx->cache->magic = (void*)if_ctx->container;
} /* _routeStatTable_container_init */

}}}
