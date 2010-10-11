#include "TextEscaper.h"

namespace eyeline {
namespace informer {

smsc::logger::Logger* TextEscaper::log_ = 0;

char* TextEscaper::unescapeText( char* text, const char* from, size_t len )
{
    if (!from) from = text;
    size_t left = len;
    char* o = text;
    if (len) {
        smsc_log_debug(log_,"unescaping text '%.*s', len=%u",len,from,len);
    } else {
        left = size_t(-1);
        smsc_log_debug(log_,"unescaping text '%s'",from);
    }
    while (left>0) {
        switch (*from) {
        case '\0' :
            if (len!=0) {
                smsc_log_warn(log_,"NULL embedded into string! len=%u left=%u from='%s'",
                              unsigned(len),unsigned(left),from-len+left);
                throw InfosmeException("NULL embedded into string at %u",unsigned(len-left));
            }
            left = 1;
            --o; // compensate
            break;
        case '\\' :
            if (!--left) {
                throw InfosmeException("escape just before EOL");
            }
            switch (*++from) {
            case '\\':
                *o = '\\';
                break;
            case 'n':
                *o = '\n';
                break;
            case '"':
                *o = '"';
                break;
            default:
                throw InfosmeException("wrong escape (\\%c) in string at %u",*from,unsigned(len ? len-left : size_t(-1)-left));
            }
            break;
        case '\n':
        case '"':
            throw InfosmeException("unescaped symbol <%s> found at %u",
                                   *from == '\n' ? "CR" : "QUOTE",
                                   unsigned((len?len:size_t(-1))-left));
        default:
            *o = *from;
        }
        ++o;
        ++from;
        --left;
    }
    *o = '\0';
    return text;
}

}
}
