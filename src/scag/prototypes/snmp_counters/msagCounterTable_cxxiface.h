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

/// it is invoked from C++
typedef msagCounterTable_list_element* (msagCounterTable_list_creator)( msagCounterTable_list_element* );
void msagCounterTable_register_list_creator( msagCounterTable_list_creator* fun );

#ifdef __cplusplus
} // extern "C" {
#endif


#endif
