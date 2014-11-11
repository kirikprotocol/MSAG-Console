/*
 * Note: this file originally auto-generated by mib2c using
 *  : generic-table-oids.m2c 17548 2009-04-23 16:35:18Z hardaker $
 *
 * $Id$
 */
#ifndef ROUTESTATTABLE_OIDS_H
#define ROUTESTATTABLE_OIDS_H

namespace scag2 {
namespace snmp {
namespace routestattable {

/* column number definitions for table routeStatTable */
#define ROUTESTATTABLE_OID            1,3,6,1,4,1,26757,2,12

#define COLUMN_ROUTESTATINDEX         1
#define COLUMN_routeStatRouteId      2
#define COLUMN_ROUTESTATACCEPTED      3
#define COLUMN_ROUTESTATREJECTED      4
#define COLUMN_ROUTESTATDELIVERED     5
#define COLUMN_ROUTESTATGWREJECTED    6
#define COLUMN_ROUTESTATFAILED        7
#define COLUMN_ROUTESTATRECEIPTOK     8
#define COLUMN_ROUTESTATRECEIPTFAILED 9

#define ROUTESTATTABLE_MIN_COL   COLUMN_ROUTESTATINDEX
#define ROUTESTATTABLE_MAX_COL   COLUMN_ROUTESTATRECEIPTFAILED
    
}}}

#endif /* ROUTESTATTABLE_OIDS_H */