#ifndef _INFORMER_RESPONSEDATA_H
#define _INFORMER_RESPONSEDATA_H

#include "util/int.h"

namespace eyeline {
namespace informer {

struct ResponseData
{
    static const size_t MSGIDSIZE = 65;

    void setMsgId( const char* msgId ) {
        if (msgId == msgid) return;
        register char* p = msgid;
        register const char* f = msgId;
        for ( size_t n = MSGIDSIZE-1; n>0; --n) {
            if (*f == '\0') break;
            *p++ = *f++;
        }
        *p = '\0';
    }

public:
    int seqNum;
    int status;
    char msgid[MSGIDSIZE];
    bool retry, receipt;
};


} // informer
} // smsc

#endif
