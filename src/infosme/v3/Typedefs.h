#ifndef _INFOSME_V3_TYPEDEFS_H
#define _INFOSME_V3_TYPEDEFS_H

#include <ctime>
#include "util/int.h"

namespace smsc {
namespace infosme {

typedef uint32_t dlvid_type;
typedef uint32_t regionid_type;
typedef uint32_t msgid_type;
typedef uint32_t msgtime_type; // simply converted from time_t
typedef  int32_t timediff_type; // simply converted from time_t
typedef int64_t  usectime_type;

/// format message time as yy-mm-dd+00:00:00, buffer must be at least 17 bytes.
char* formatMsgTime( char* buf, msgtime_type theTime );

/// get current microtime
usectime_type currentTimeMicro();

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

} // infosme
} // smsc

#endif
