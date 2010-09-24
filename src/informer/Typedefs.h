#ifndef _INFORMER_TYPEDEFS_H
#define _INFORMER_TYPEDEFS_H

#include <ctime>
#include "util/int.h"

namespace smsc {
namespace informer {

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

namespace MsgState {
static const uint8_t input = 0;
static const uint8_t process = 1;
static const uint8_t sent = 2;
static const uint8_t delivered = 3;
static const uint8_t expired = 4;
static const uint8_t failed = 5;
static const uint8_t retry = 6;
const char* toString( uint8_t state );
}


struct DlvRegMsgId
{
    dlvid_type    dlvId;
    regionid_type regId;
    msgid_type    msgId;
};

} // informer
} // smsc

#endif
