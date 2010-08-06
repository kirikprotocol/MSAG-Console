#ifndef _SCAG_PROTOTYPES_INFOSME_MESSAGE_HPP
#define _SCAG_PROTOTYPES_INFOSME_MESSAGE_HPP

#include <list>
#include <cassert>
#include "util/int.h"
#include "core/buffers/FixedLengthString.hpp"

namespace scag2 {
namespace prototypes {
namespace infosme {

typedef uint32_t dlvid_type;
typedef uint32_t regionid_type;

typedef uint32_t msgid_type;
typedef uint64_t msgpos_type;
typedef uint32_t fpos_type;
typedef int32_t  msgtime_type;  // counted in seconds since 2010-01-01+00:00:00 GMT
typedef int64_t  usectime_type;

/// return the time of 2010-02-01+00:00:00
void initgmt20100101();

extern time_t gmt20100101_;

inline msgtime_type getCurrentTime() {
    assert(gmt20100101_);
    return msgtime_type(time(0) - gmt20100101_);
}

/// format message time as yy-mm-dd+00:00:00, buffer must be at least 17 bytes.
char* formatMsgTime( char* buf, msgtime_type theTime );

/*
namespace Pos {
static const fpos_type   fposNotReady = fpos_type(-1);
static const fpos_type   fposEof = fpos_type(0);

static const msgpos_type notReadyPos = 0xffffffffULL;
static const msgpos_type fileFactor = 0x100000000ULL;
}
 */

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


struct DlvMsgId
{
    dlvid_type    dlvId;
    regionid_type regId;
    msgid_type    msgId;
};


// 8+24+4+4+4+4+1+1 = 32+32+2 = 66 => 72
struct Message
{
    static const size_t USERDATA_LENGTH = 24;

    Message() {}

    uint64_t      subscriber; // not changed (constant)
    msgid_type    msgId;      // unique message id (constant)
    msgtime_type  lastTime;   // lasttime / filenumber (for new messages)
    msgtime_type  timeLeft;   // ttl / filepos (for new messages)
    uint32_t      textId;
    smsc::core::buffers::FixedLengthString<USERDATA_LENGTH> userData; // (constant)
    uint8_t       state;

    /// conversion
    unsigned char* toBuf( uint16_t version, unsigned char* buf );
    const unsigned char* fromBuf( uint16_t version, const unsigned char* buf );

    /// textual
    char* printToBuf( uint16_t version, char* buf );
};

// a structure which is kept in the list in operational storage.
// it contains the message itself and the 'locked' flag.
struct MessageData
{
    MessageData() : locked(false) {}
    Message          msg;
    volatile uint8_t locked;
};

// a list of messages
typedef std::list< MessageData >  MessageList;
typedef MessageList::iterator     MsgIter;

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_INFOSMEINPUTSTORAGE_HPP */
