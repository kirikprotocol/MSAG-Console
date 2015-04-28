/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 18978 $ of $
 *
 * $Id$
 */
/** @ingroup interface: Routines to interface to Net-SNMP
 *
 * \warning This code should not be modified, called directly,
 *          or used to interpret functionality. It is subject to
 *          change at any time.
 *
 * @{
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
#ifndef ROUTEUSSDTABLE_INTERFACE_H
#define ROUTEUSSDTABLE_INTERFACE_H


#include "routeStatTable.hpp"

/* ********************************************************************
 * Table declarations
 */

namespace scag2 {
namespace snmp {
namespace routeussdtable {


/* PUBLIC interface initialization routine */
void _routeStatTable_initialize_interface(routeStatTable_registration_ptr user_ctx, u_long flags);

routeStatTable_rowreq_ctx* routeStatTable_allocate_rowreq_ctx(void);
void routeStatTable_release_rowreq_ctx(routeStatTable_rowreq_ctx* rowreq_ctx);

int routeStatTable_index_to_oid(netsnmp_index *oid_idx, routeStatTable_mib_index *mib_idx);
int routeStatTable_index_from_oid(netsnmp_index *oid_idx, routeStatTable_mib_index *mib_idx);

/*
 * access to certain internals. use with caution!
 */
void routeStatTable_valid_columns_set(netsnmp_column_info *vc);

}}}

#endif /* ROUTEUSSDTABLE_INTERFACE_H */
