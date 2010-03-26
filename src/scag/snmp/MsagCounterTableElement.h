#ifndef _SCAG_SNMP_MSAGCOUNTERTABLEELEMENT_H
#define _SCAG_SNMP_MSAGCOUNTERTABLEELEMENT_H

// NOTE: this is C header

// It defines only an intermediate structure which is used to feed
// counter data into snmp counter system.

// The purpose of the header is to decouple the feeder from net-snmp completely!

#include "util/int.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MsagCounterTableElement
{
    char     name[49]; // +1 for trailing zero
    int      namelen;
    uint64_t value;
    uint8_t  enabled;
    // should we have user context attached?
    // void* userctx;
    struct MsagCounterTableElement* next;
} MsagCounterTableElement;
    

/// The creator take the previously defined table,
/// updates the values of its elements,
/// disables those elements that became inactive,
/// and adds new elements if necessary.
typedef MsagCounterTableElement* (msagCounterTable_creator_t)( MsagCounterTableElement* );
typedef void (msagCounterTable_destructor_t)( MsagCounterTableElement* );

/*
void msagCounterTable_init( msagCounterTable_creator *creator,
                            msagCounterTable_creator *destructor,
                            const uint32_t           *tableOid,
                            int                       tableOidLen,
                            int                       cacheTimeout );
 */
                            
#ifdef __cplusplus
}
#endif

#endif /* _SCAG_SNMP_MSAGCOUNTERTABLEELEMENT_H */
