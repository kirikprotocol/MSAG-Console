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
    
    /* not needed anymore - using per smsc policy
    static bool wantRetry( int stat ) {
        if ( stat != smsc::system::Status::OK && 
             !smsc::system::Status::isErrorPermanent(stat) ) {
            return true;
        } else {
            return false;
        }
    }
     */
};


struct ResponseTimer
{
    msgtime_type endTime;
    int          seqNum;
};


struct ReceiptData
{
    DlvRegMsgId  drmId;
    ReceiptId    rcptId;
    msgtime_type endTime;
    int          status;
    uint16_t     responded; // 0 -- not responed, >0 -- responded (nchunks)
    bool         retry;
};

} // informer
} // smsc

#endif
