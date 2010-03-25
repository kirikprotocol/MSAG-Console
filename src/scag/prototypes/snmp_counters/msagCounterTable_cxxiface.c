#include "msagCounterTable_cxxiface.h"
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/types.h"

static msagCounterTable_list_element* thelist = 0;
msagCounterTable_list_creator* creator = 0;

static oid *tableOid = 0;
static int tableOidLen = 0;
static int cacheTimeout = 10;

msagCounterTable_list_element *msagCounterTable_get_counter_list( void )
{
    if (creator) {
        thelist = (*creator)(thelist);
    }
    return thelist;
}


void msagCounterTable_release_counter_list( msagCounterTable_list_element* list )
{
}


int msagCounterTable_get_cache_timeout( void ) 
{
    return cacheTimeout;
}

void msagCounterTable_set_cache_timeout( int seconds )
{
    cacheTimeout = seconds;
}


void msagCounterTable_register_list_creator( msagCounterTable_list_creator* c )
{
    creator = c;
}


void* msagCounterTable_get_table_oid( int* oidlen )
{
    if (oidlen) *oidlen = tableOidLen;
    return tableOid;
}

void msagCounterTable_set_table_oid( const uint32_t* theoid, int oidlen )
{
    const uint32_t* p = theoid;
    const uint32_t* endp = theoid + oidlen;
    oid* op;

    tableOidLen = oidlen;
    if (tableOid) free(tableOid);
    printf("counter table OID[%u]:",(unsigned)tableOidLen);
    op = tableOid = (oid*) malloc(oidlen*sizeof(oid));
    for ( ; p != endp; ++p  ) {
        *op = (oid)*p;
        printf(" %u",(unsigned)*op);
        ++op;
    }
    printf("\n");
}
