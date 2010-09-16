#ifndef _INFOSME_V3_MESSAGE_H
#define _INFOSME_V3_MESSAGE_H

#include "Typedefs.h"
#include "core/buffers/FixedLengthString.hpp"
#include <list>

namespace smsc {
namespace infosme {

// 8+24+4+4+4+4+1+1 = 32+32+2 = 66 => 72
struct Message
{
    static const size_t USERDATA_LENGTH = 24;

    Message() {}

    uint64_t         subscriber; // not changed (constant)
    msgid_type       msgId;      // unique message id (constant)
    msgtime_type     lastTime;   // lasttime / filenumber (for new messages)
    int32_t          timeLeft;   // ttl / filepos (for new messages)
    uint32_t         textId;
    smsc::core::buffers::FixedLengthString<USERDATA_LENGTH> userData; // (constant)
    uint8_t          state;

    /// conversion (for I/O)
    unsigned char* toBuf( uint16_t version, unsigned char* buf );
    const unsigned char* fromBuf( uint16_t version, const unsigned char* buf );

    /// textual
    char* printToBuf( size_t bufsize, char* buf );
};

// a structure which is kept in the list in operational storage.
// it contains the message itself and the 'locked' flag.
struct MessageLocker
{
    MessageLocker() : locked(false) {}
    Message          msg;
    volatile uint8_t locked;
};

// a list of messages
typedef std::list< MessageLocker >  MessageList;
typedef MessageList::iterator       MsgIter;

}
}

#endif
