#ifndef _INFORMER_MESSAGE_H
#define _INFORMER_MESSAGE_H

#include <list>
#include "MessageText.h"
#include "informer/io/Typedefs.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace informer {

class ToBuf;
class FromBuf;

// 8+8+4+4+16+24+1 = 16+24+24+1 = 40+25 = 65 => 72
struct Message
{
    static const size_t USERDATA_LENGTH = 24;

    Message() {}
    Message( const Message& m ) :
    subscriber(m.subscriber),
    msgId(m.msgId),
    lastTime(m.lastTime),
    timeLeft(m.timeLeft),
    text(m.text),
    userData(m.userData),
    state(m.state) {}

    uint64_t         subscriber; // not changed (constant)
    msgid_type       msgId;      // unique message id (constant)
    msgtime_type     lastTime;   // lasttime / filenumber (for new messages)
    timediff_type    timeLeft;   // ttl / filepos (for new messages)
    MessageTextPtr   text;       // message text
    smsc::core::buffers::FixedLengthString<USERDATA_LENGTH> userData; // (constant)
    uint8_t          state;

    /// conversion (for I/O)
    ToBuf& toBuf( uint16_t version, ToBuf& tb ) const;
    FromBuf& fromBuf( uint16_t version, FromBuf& fb );

    inline bool isTextUnique() const {
        return (text->getTextId() <= 0);
    }
    // textual representation
    // FIXME: should we add 'version' arg?
    // char* printToBuf( size_t bufsize, char* buf ) const;
};


inline uint64_t subscriberToAddress( uint64_t subsc, uint8_t& ton, uint8_t& npi )
{
    register uint8_t tonnpi = uint8_t(subsc >> 56);
    ton = tonnpi >> 4;
    npi = tonnpi & 0xf;
    return subsc & 0xffffffffffffffULL;
}

inline uint64_t addressToSubscriber( uint8_t ton, uint8_t npi, uint64_t value )
{
    return ( uint64_t((ton & 0xf << 4) | (npi & 0xf)) << 56 ) | (value & 0xffffffffffffffULL);
}

// a structure which is kept in the list in operational storage.
// it contains the message itself and the 'locked' flag.
struct MessageLocker
{
    MessageLocker() : locked(0) {}
    MessageLocker(const MessageLocker& mlk) : msg(mlk.msg), locked(mlk.locked) {}
    Message          msg;
    volatile regionid_type locked;
};

// a list of messages
typedef std::list< MessageLocker >  MessageList;
typedef MessageList::iterator       MsgIter;

}
}

#endif
