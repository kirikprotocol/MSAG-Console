#include "msagCounterTable_cxxiface.h"

static msagCounterTable_list_element* thelist = 0;
msagCounterTable_list_creator* creator = 0;

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
    return 10;
}


void msagCounterTable_register_list_creator( msagCounterTable_list_creator* c )
{
    creator = c;
}
