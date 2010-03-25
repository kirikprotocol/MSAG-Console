#ifndef MSAGCOUNTERTABLE_CXXIFACE_H
#define MSAGCOUNTERTABLE_CXXIFACE_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msagCounterTable_counter {

    char     name[48];
    int      namelen;
    uint64_t value;

} msagCounterTable_counter;

typedef struct msagCounterTable_list_element {
    msagCounterTable_counter data;
    struct msagCounterTable_list_element *next;
} msagCounterTable_list_element;

/// obtain the current list of elements, externally owned.
msagCounterTable_list_element *msagCounterTable_get_counter_list( void );

/// release the list of elements
void msagCounterTable_release_counter_list( msagCounterTable_list_element* list );

/// obtain the current value of cache timeout (seconds)
int msagCounterTable_get_cache_timeout( void );

/// actual type of return value depends on netsnmp-config
void* msagCounterTable_get_table_oid( int* oidlen );

/// functions which are invoked from C++
void msagCounterTable_set_cache_timeout( int seconds );

typedef msagCounterTable_list_element* (msagCounterTable_list_creator)( msagCounterTable_list_element* );
void msagCounterTable_register_list_creator( msagCounterTable_list_creator* fun );

void msagCounterTable_set_table_oid( const uint32_t* theoid, int oidlen );

#ifdef __cplusplus
} // extern "C" {
#endif


#endif
