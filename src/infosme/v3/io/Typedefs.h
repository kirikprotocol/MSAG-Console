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

/// format message time as yyyymmddHHMMSS, buffer must be at least 15 bytes.
char* formatMsgTime( char* buf, msgtime_type theTime );

/// get current microtime
usectime_type currentTimeMicro();

/// some constants
static const unsigned tuPerSec = 1000000U;
static const unsigned maxScoreIncrement = 10000U;
static const unsigned flipTimePeriod = 1000*tuPerSec;

typedef enum {
    MSGSTATE_INPUT = 0,
    MSGSTATE_PROCESS = 1,
    MSGSTATE_SENT = 2,
    MSGSTATE_DELIVERED = 3,
    MSGSTATE_EXPIRED = 4,
    MSGSTATE_FAILED = 5,
    MSGSTATE_RETRY = 6
} MsgState;
const char* msgStateToString( MsgState state );

typedef enum {
    DLVSTATE_PAUSED = 0,
    DLVSTATE_PLANNED = 1,
    DLVSTATE_ACTIVE = 2,
    DLVSTATE_FINISHED = 3,
    DLVSTATE_CANCELLED = 4
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

} // informer
} // smsc

#endif
