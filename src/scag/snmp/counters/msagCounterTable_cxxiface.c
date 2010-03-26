#include "msagCounterTable_cxxiface.h"
#include "msagCounterTable.h"

static oid msagoid[] = { 1,3,6,1,4,1,26757,2,4 };
static oid pvssoid[] = { 1,3,6,1,4,1,26757,3,4 };

void initMsagCounterTable( msagCounterTable_creator_t*    creator,
                           msagCounterTable_destructor_t* destructor,
                           int                            cacheTimeout )
{
    if ( ! creator || msagCounterTable_creator ) abort();
    msagCounterTable_oid = msagoid;
    msagCounterTable_oid_size = sizeof(msagoid)/sizeof(msagoid[0]);
    msagCounterTable_creator = creator;
    msagCounterTable_destructor = destructor;
    msagCounterTable_cacheTimeout = cacheTimeout;
    // init_agent("msagCounterTable");  // init agent lib, must be done externally
    init_msagCounterTable();
    // init_snmp("msagCounterTable");   // reading config
}

void shutdownMsagCounterTable()
{
    // snmp_shutdown("msagCounterTable");
    msagCounterTable_oid = 0;
    msagCounterTable_oid_size = 0;
    msagCounterTable_creator = 0;
}



void initPvssCounterTable( msagCounterTable_creator_t*    creator,
                           msagCounterTable_destructor_t* destructor,
                           int                            cacheTimeout )
{
    if ( ! creator || msagCounterTable_creator ) abort();
    msagCounterTable_oid = pvssoid;
    msagCounterTable_oid_size = sizeof(pvssoid)/sizeof(pvssoid[0]);
    msagCounterTable_creator = creator;
    msagCounterTable_destructor = destructor;
    msagCounterTable_cacheTimeout = cacheTimeout;
    // init_agent("pvssCounterTable"); // init agent lib, must be done externally
    init_msagCounterTable();
    // init_snmp("pvssCounterTable");  // reading config
}

void shutdownPvssCounterTable()
{
    // snmp_shutdown("pvssCounterTable");
    msagCounterTable_oid = 0;
    msagCounterTable_oid_size = 0;
    msagCounterTable_creator = 0;
}
