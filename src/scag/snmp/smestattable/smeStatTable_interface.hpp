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
#ifndef SMESTATTABLE_INTERFACE_H
#define SMESTATTABLE_INTERFACE_H


#include "smeStatTable.hpp"

/* ********************************************************************
 * Table declarations
 */

namespace scag2 {
namespace snmp {
namespace smestattable {


/* PUBLIC interface initialization routine */
void _smeStatTable_initialize_interface(smeStatTable_registration_ptr user_ctx, u_long flags);

smeStatTable_rowreq_ctx* smeStatTable_allocate_rowreq_ctx(void);
void smeStatTable_release_rowreq_ctx(smeStatTable_rowreq_ctx* rowreq_ctx);

int smeStatTable_index_to_oid(netsnmp_index *oid_idx, smeStatTable_mib_index *mib_idx);
int smeStatTable_index_from_oid(netsnmp_index *oid_idx, smeStatTable_mib_index *mib_idx);

/*
 * access to certain internals. use with caution!
 */
void smeStatTable_valid_columns_set(netsnmp_column_info *vc);

}}}

#endif /* SMESTATTABLE_INTERFACE_H */
