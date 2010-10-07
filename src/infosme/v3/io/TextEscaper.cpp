#include "TextEscaper.h"

namespace eyeline {
namespace informer {

smsc::logger::Logger* TextEscaper::log_ = 0;

char* TextEscaper::unescapeText( char* text, const char* from, size_t len )
{
    if (!from) from = text;
    size_t left = len;
    if (!left) left = size_t(-1);
    char* o = text;
    if (len) {
        smsc_log_debug(log_,"unescaping text '%.*s'",len,from);
    } else {
        smsc_log_debug(log_,"unescaping text '%s'",from);
    }
    while (left>0) {
        switch (*from) {
        case '\0' :
            if (len!=0) {
                throw InfosmeException("NULL embedded into string at %u",unsigned(len-left));
            }
            left = 1;
            --o; // compensate
            break;
        case '\\' :
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
                throw InfosmeException("wrong escape (\\%c) in string at %u",*from,unsigned(len-left));
            }
            break;
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
