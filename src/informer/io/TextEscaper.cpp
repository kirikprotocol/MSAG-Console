#include "TextEscaper.h"

namespace eyeline {
namespace informer {

smsc::logger::Logger* TextEscaper::log_ = 0;

void TextEscaper::escapeText( TmpBufBase<char>& buf, const char* from,
                              size_t len, bool quoteAsCsv )
{
    size_t extra = len/10 + 10;
    size_t cursize = buf.GetPos() + len + extra;
    buf.reserve(cursize);
    const char* p = from;
    char* to = buf.GetCurPtr();
    // smsc_log_debug(log_,"escaping text '%.*s'",len,from);
    while (len>0) {
        switch (*p) {
        case '\0' :
            throw InfosmeException(EXC_IOERROR,"NULL embedded into string at %u",unsigned(p-from));
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
            *to = quoteAsCsv ? '"' : '\\';
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
    // smsc_log_debug(log_,"after escape '%s'",buf.get());
}


char* TextEscaper::unescapeText( char* text, const char* from, size_t len,
                                 bool quoteAsCsv )
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
                throw InfosmeException(EXC_IOERROR,"NULL embedded into string at %u",unsigned(len-left));
            }
            left = 1;
            --o; // compensate
            break;
        case '\\' :
            if (!--left) {
                throw InfosmeException(EXC_IOERROR,"escape just before EOL");
            }
            switch (*++from) {
            case '\\':
                *o = '\\';
                break;
            case 'n':
                *o = '\n';
                break;
            case '"':
                if (quoteAsCsv) {
                    throw InfosmeException(EXC_IOERROR,"quote escaped via backslash at %u",unsigned(len ? len-left : size_t(-1)-left));
                }
                *o = '"';
                break;
            default:
                throw InfosmeException(EXC_IOERROR,"wrong escape (\\%c) in string at %u",*from,unsigned(len ? len-left : size_t(-1)-left));
            }
            break;
        case '"':
            if (quoteAsCsv) {
                if (!--left) {
                    throw InfosmeException(EXC_IOERROR,"quote just before EOL");
                }
                if (*++from != '"') {
                    throw InfosmeException(EXC_IOERROR,"unescaped quote in string at %u",unsigned(len?len-left:size_t(-1)-len)-1);
                }
                *o = '"';
                break;
            }
        case '\n':
            throw InfosmeException(EXC_IOERROR,
                                   "unescaped symbol <%s> found at %u",
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
