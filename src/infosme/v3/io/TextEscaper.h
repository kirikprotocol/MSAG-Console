#ifndef _INFORMER_TEXTESCAPER_H
#define _INFORMER_TEXTESCAPER_H

#include <cstring>
#include "core/buffers/TmpBuf.hpp"
#include "informer/io/InfosmeException.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

template <size_t N> struct TextEscaper
{
public:
    TextEscaper() {
        if (!log_) {
            log_ = smsc::logger::Logger::getInstance("escaper");
        }
    }

    void escape( smsc::core::buffers::TmpBuf<char,N>& buf,
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
private:
    static smsc::logger::Logger* log_;
};

template <size_t N> smsc::logger::Logger* TextEscaper<N>::log_ = 0;

template <size_t N> 
    void escapeText( smsc::core::buffers::TmpBuf<char,N>& buf,
                     const char* from, size_t len ) 
{
    TextEscaper<N> te;
    te.escape(buf,from,len);
}


} // informer
} // smsc

#endif
