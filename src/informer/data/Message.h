#ifndef _INFORMER_MESSAGE_H
#define _INFORMER_MESSAGE_H

#include <list>
#include "MessageText.h"
#include "MessageFlags.h"
#include "informer/io/Typedefs.h"
#include "informer/io/InfosmeException.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace informer {

// 8+8+4+4+8+4+24+2+1 = 16+16+24+3 = 32+27 = 59 + (4bytes/locker) => 64 bytes
struct Message
{
    static const size_t USERDATA_LENGTH = 24;

    msgid_type       msgId;      // unique message id (constant)          8
    personid_type    subscriber; // not changed (constant)                8
    smsc::core::buffers::FixedLengthString<USERDATA_LENGTH> userData; // 24 (constant)
    MessageText      text;       // message text                         16
    MessageFlags     flags;      // message flags                        16
    msgtime_type     lastTime;   // lasttime / filenumber                 4
    timediff_type    timeLeft;   // ttl / filepos (for new messages)      4
    uint16_t         retryCount; //                                       2
    uint8_t          state;      //                                       1
                                 // total size:                          71 

    inline void swap( Message& m ) {
        std::swap(subscriber,m.subscriber);
        std::swap(msgId,m.msgId);
        std::swap(lastTime,m.lastTime);
        std::swap(timeLeft,m.timeLeft);
        std::swap(userData,m.userData);
        text.swap(m.text);
        std::swap(retryCount,m.retryCount);
        flags.swap(m.flags);
        std::swap(state,m.state);
    }

    inline void setUserData( const char* userDataValue ) {
        if ( strlen(userDataValue) >= USERDATA_LENGTH ) {
            throw InfosmeException(EXC_BADNAME, "too long userdata '%s'",userDataValue);
        }
        userData = userDataValue;
    }

    static const uint16_t maxRetryCount = 0xffff;
};


// a structure which is kept in the list in operational storage.
// it contains the message itself and the 'serial' flag.
// Serial flag has several purposes:
// 1. At creation stage in InputStorage it keeps the region id
//    to be sorted by regions;
// 2. At opstore stage it keeps the current serial number of the storage
//    journal file.  If the serial number of the message in memory is not equal
//    to the current serial number of the storage journal file, then the message
//    should be fully written, otherwise only the delta may be written.
//    The main idea is to reduce the size of the written data.
//    To fulfill the task there are the serial can take the following values:
//    0 -- not equal to any serial (used at input to guarantee the full write);
//    regionid_type(-1) -- locked;
//    values >= regionid_type(-255) -- reserved for future use.
//    values < regionid_type(-255) -- valid value for the serial number.
struct MessageLocker
{
    static const regionid_type nullSerial = regionid_type(0);
    static const regionid_type lockedDelete = regionid_type(-1);
    static const regionid_type lockedSerial = regionid_type(-2);
    MessageLocker() : serial(nullSerial), users(0) {}
    MessageLocker(const MessageLocker& mlk) :
    msg(mlk.msg), serial(mlk.serial), users(mlk.users) {}
    Message                msg;
    volatile regionid_type serial;
    volatile uint8_t       users;  // a number of user thread for this iterator
};

// a list of messages
typedef std::list< MessageLocker >  MessageList;
typedef MessageList::iterator       MsgIter;

}
}

#endif
