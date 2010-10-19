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
};


struct ResponseData
{
public:
    int         seqNum; // 0 if receipt, any other if response
    int         status;
    ReceiptId   rcptId;
    bool        retry;
    
    static bool wantRetry( int stat ) {
        if ( stat != smsc::system::Status::OK && 
             !smsc::system::Status::isErrorPermanent(stat) ) {
            return true;
        } else {
            return false;
        }
    }
};


struct ResponseTimer
{
    msgtime_type endTime;
    int          seqNum;
};


struct ReceiptData
{
    DlvRegMsgId drmId;
    ReceiptId   rcptId;
    int         status;
    bool        responded;
    bool        retry;
};


struct ReceiptTimer
{
    msgtime_type endTime;
    ReceiptId    rcptId;
};

} // informer
} // smsc

#endif
