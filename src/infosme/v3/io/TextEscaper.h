#ifndef _INFORMER_TEXTESCAPER_H
#define _INFORMER_TEXTESCAPER_H

#include <cstring>
#include "core/buffers/TmpBuf.hpp"
#include "informer/io/InfosmeException.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

struct TextEscaper
{
public:
    TextEscaper() {
        if (!log_) {
            log_ = smsc::logger::Logger::getInstance("escaper");
        }
    }

    template <size_t N>
    void escapeText( smsc::core::buffers::TmpBuf<char,N>& buf,
                     const char* from, size_t len )
    {
        size_t extra = len/10 + 10;
        size_t cursize = buf.GetPos() + len + extra;
        buf.reserve(cursize);
        const char* p = from;
        char* to = buf.GetCurPtr();
        smsc_log_debug(log_,"escaping text '%.*s'",len,from);
        while (len>0) {
            switch (*p) {
            case '\0' :
                throw InfosmeException("NULL embedded into string at %u",unsigned(p-from));
                break;
            case '\\' :
                *to = '\\';
                *++to = '\\';
                --extra;
                break;
            case '\n' :
                *to = '\\';
                *++to = 'n';
                --extra;
                break;
            case '"' :
                *to = '\\';
                *++to = '"';
                --extra;
                break;
            default:
                *to = *p;
                goto nocheck;
            }
            // check extra
            if (extra<2) {
                buf.SetPos(to - buf.get());
                cursize += 50;
                extra += 50;
                buf.reserve(cursize);
                to = buf.GetCurPtr();
            }
nocheck:
            ++to;
            ++p;
            --len;
        }
        *to = '\0'; // trailer
        buf.SetPos(to-buf.get());
        smsc_log_debug(log_,"after escape '%s'",buf.get());
    }

    char* unescapeText( char* text, const char* from = 0, size_t len = 0 );

private:
    static smsc::logger::Logger* log_;
};

template <size_t N> 
    void escapeText( smsc::core::buffers::TmpBuf<char,N>& buf,
                     const char* from, size_t len ) 
{
    TextEscaper te;
    te.escapeText(buf,from,len);
}


inline char* unescapeText( char* text, const char* from = 0, size_t len = 0 ) {
    TextEscaper te;
    return te.unescapeText(text,from,len);
}

} // informer
} // smsc

#endif
