#ifndef MSAGCOUNTERTABLE_CXXIFACE_H
#define MSAGCOUNTERTABLE_CXXIFACE_H

#include "scag/snmp/MsagCounterTableElement.h"

#ifdef __cplusplus
extern "C" {
#endif

void initMsagCounterTable( msagCounterTable_creator_t*    creator,
                           msagCounterTable_destructor_t* destructor,
                           int                            cacheTimeout );

void initPvssCounterTable( msagCounterTable_creator_t*    creator,
                           msagCounterTable_destructor_t* destructor,
                           int                            cacheTimeout );

void shutdownMsagCounterTable( void );
void shutdownPvssCounterTable( void );

#ifdef __cplusplus
} // extern "C" {
#endif

#endif
