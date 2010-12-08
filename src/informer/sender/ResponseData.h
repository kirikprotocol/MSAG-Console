#ifndef _INFORMER_RESPONSEDATA_H
#define _INFORMER_RESPONSEDATA_H

#include "util/int.h"
#include "system/status.h"

namespace eyeline {
namespace informer {

struct ReceiptId
{
    static const size_t MSGIDSIZE = 65; // with trailing \0
    void setMsgId( const char* msgid ) {
        if (msgId == msgid) return;
        register char* p = msgId;
        register const char* f = msgid;
        for ( size_t n = MSGIDSIZE-1; n>0; --n) {
            if (*f == '\0') break;
            *p++ = *f++;
        }
        *p = '\0';
    }
    char msgId[MSGIDSIZE];

    inline bool operator == ( const ReceiptId& r ) const {
        register const char* b = msgId;
        register const char* p = r.msgId;
        if (b == p) return true;
        while (true) {
            if ( *b != *p ) return false;
            if ( !*b ) return true;
            ++b;
            ++p;
        }
        return true;
    }
};


struct ResponseData
{
public:
    int         seqNum; // 0 if receipt, any other if response
    int         status;
    ReceiptId   rcptId;
    /// if we need to attempt to generate retry:
    ///  - response - always true;
    ///  - receipt - deduced from receipt message_state (final/non-final).
    bool        retry;
};


struct ReceiptData
{
    DlvRegMsgId  drmId;
    ReceiptId    rcptId;
    msgtime_type endTime;
    int          status;
    uint16_t     responded; // 0 -- not responded, >0 -- responded (nchunks)
    bool         retry;
};

} // informer
} // smsc

#endif
