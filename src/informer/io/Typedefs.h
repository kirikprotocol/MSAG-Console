#ifndef _INFORMER_TYPEDEFS_H
#define _INFORMER_TYPEDEFS_H

#include <ctime>
#include <string>
#include <cstdio>
#include "util/int.h"

namespace eyeline {
namespace informer {

typedef unsigned long long ulonglong;

typedef uint32_t dlvid_type;
typedef uint32_t regionid_type;
typedef uint64_t msgid_type;
typedef uint32_t msgtime_type; // simply converted from time_t
typedef  int32_t timediff_type; // simply converted from time_t
typedef int64_t  usectime_type;
typedef uint64_t personid_type;

/// predefined region ids
static const regionid_type defaultRegionId = regionid_type(0);
static const regionid_type anyRegionId = regionid_type(-1);

/// format message time as yyyymmddHHMMSS, buffer must be at least 15 bytes.
/// return number of bytes printed (not including trailing \0)
int formatMsgTime( char* buf, msgtime_type theTime, struct tm* tmb = 0 );

/// scanf message time converted by formatMsgTime
msgtime_type scanfMsgTime( ulonglong tmbuf );

/// get current microtime
usectime_type currentTimeMicro();

/// some constants
static const unsigned tuPerSec = 1000000U;
static const unsigned maxScoreIncrement = 10000U;
static const unsigned flipTimePeriod = 1000*tuPerSec;

inline uint64_t subscriberToAddress( personid_type subsc, uint8_t& len, uint8_t& ton, uint8_t& npi )
{
    register uint16_t lentonnpi = uint16_t(subsc >> 52);
    len = uint8_t(lentonnpi >> 8) + 1;   // 1..16 by construction
    ton = uint8_t(lentonnpi >> 4) & 0xf; // 0..15 by construction
    npi = uint8_t(lentonnpi & 0xf);      // 0..15 by construction
    return uint64_t(subsc & 0xfffffffffffffULL);
}

inline personid_type addressToSubscriber( uint8_t len, uint8_t ton, uint8_t npi, uint64_t value )
{
    return ( uint64_t( (uint16_t((len-1) & 0xf) << 8) | ((ton & 0xf) << 4) | (npi & 0xf)) << 52 ) | (value & 0xfffffffffffffULL);
}

typedef enum {
    MSGSTATE_INPUT = 1,
    MSGSTATE_PROCESS = 2,
    MSGSTATE_TAKEN = 3,     // actually it is technological state (not seen externally)
    MSGSTATE_SENT = 4,
    MSGSTATE_RETRY = 5,     // non-final
    MSGSTATE_DELIVERED = 6,
    MSGSTATE_FINAL = MSGSTATE_DELIVERED,
    MSGSTATE_EXPIRED = 7,
    MSGSTATE_FAILED = 8,
    MSGSTATE_MAX = MSGSTATE_FAILED
} MsgState;
const char* msgStateToString( MsgState state );

typedef enum {
    DLVSTATE_PAUSED = 1,
    DLVSTATE_PLANNED = 2,
    DLVSTATE_ACTIVE = 3,
    DLVSTATE_FINISHED = 4,
    DLVSTATE_CANCELLED = 5,
    DLVSTATE_MAX = DLVSTATE_CANCELLED
} DlvState;

const char* dlvStateToString( DlvState state );


struct DlvRegMsgId
{
    dlvid_type    dlvId;
    regionid_type regId;
    msgid_type    msgId;
};


template <typename RegIter> std::string formatRegionList( RegIter begin, RegIter end )
{
    std::string s;
    s.reserve(std::distance(begin,end)*10);
    for ( RegIter i = begin; i != end; ++i ) {
        char buf[20];
        sprintf(buf," R=%u",unsigned(*i));
        s.append(buf);
    }
    return s;
}


/// buf must be at least 40 bytes
/// @return pointer to trailing \0.
char* makeDeliveryPath( dlvid_type dlvId, char* buf );

} // informer
} // smsc

#endif
