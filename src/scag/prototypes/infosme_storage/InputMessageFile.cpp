#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "InputMessageFile.h"
#include "util/Exception.hpp"

namespace scag2 {
namespace prototypes {
namespace infosme {

int InputMessageFile::getNextMessage( PosMessage& msg )
{
    if ( cache_.Pop(msg) ) {
        return 1;
    }
    // otherwise trying to read buffer
    char buf[MAXINPUTMESSAGELEN+BUFLEN];
    char* bufptr = buf+MAXINPUTMESSAGELEN;
    uint32_t offset;
    const ssize_t bytesRead = df_.readBuffer(bufptr,BUFLEN,offset);
    if ( bytesRead == -1 ) {
        // not ready
        return -1;
    } else if ( bytesRead == 0 ) {
        // eof
        return 0;
    }

    if (buflen_) {
        // copy the rest of previous buffer to the beginning
        bufptr = bufptr-buflen_;
        memcpy(bufptr,buf_,buflen_);
    }
    buflen_ += bytesRead;
    while (true) {
        char* endptr = static_cast<char*>( memchr(bufptr,'\n',buflen_) );
        if (!endptr) {
            // not found, saving the final chunk
            memcpy(buf_,bufptr,buflen_);
            break;
        }
        *endptr = '\0';
        PosMessage pm;
        pm.pos = offset;
        if ( readRecord(pm.msg,bufptr,endptr-bufptr) ) {
            cache_.Push(pm);
        } else {
            throw smsc::util::Exception("cannot parse message at %u",pm.pos);
        }
        ++endptr;
        offset += endptr-bufptr;
        buflen_ -= endptr-bufptr;
        bufptr = endptr;
        if (buflen_==0) break;
    }
    // cache is filled
    if ( cache_.Pop(msg) ) {
        return 1;
    }
    return -1;
}

}
}
}
