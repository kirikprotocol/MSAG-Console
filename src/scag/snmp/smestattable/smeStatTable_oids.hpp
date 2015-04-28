/*
 * Note: this file originally auto-generated by mib2c using
 *  : generic-table-oids.m2c 17548 2009-04-23 16:35:18Z hardaker $
 *
 * $Id$
 */
#ifndef SMESTATTABLE_OIDS_H
#define SMESTATTABLE_OIDS_H

namespace scag2 {
namespace snmp {
namespace smestattable {

/* column number definitions for table smeStatTable */
#define SMESTATTABLE_OID            1,3,6,1,4,1,26757,2,20

#define COLUMN_SMESTATINDEX         1
#define COLUMN_SMESTATSYSTEMID      2
#define COLUMN_SMESTATACCEPTED      3
#define COLUMN_SMESTATREJECTED      4
#define COLUMN_SMESTATDELIVERED     5
#define COLUMN_SMESTATGWREJECTED    6
#define COLUMN_SMESTATFAILED        7
#define COLUMN_SMESTATRECEIPTOK     8
#define COLUMN_SMESTATRECEIPTFAILED 9

#define SMESTATTABLE_MIN_COL   COLUMN_SMESTATINDEX
#define SMESTATTABLE_MAX_COL   COLUMN_SMESTATRECEIPTFAILED
    
}}}

#endif /* SMESTATTABLE_OIDS_H */
