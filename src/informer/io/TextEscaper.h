#ifndef _INFORMER_TEXTESCAPER_H
#define _INFORMER_TEXTESCAPER_H

#include <cstring>
#include "TmpBuf.h"
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

    /// escape text into TmpBuf
    /// @param from the text
    /// @param len is the number of bytes in text
    /// @param quoteAsCSV if true then encode quote as "", otherwise as \"
    void escapeText( TmpBufBase<char>& buf, const char* from,
                     size_t len, bool quoteAsCsv = false );

    char* unescapeText( char* text, const char* from = 0,
                        size_t len = 0, bool quoteAsCsv = false );

private:
    static smsc::logger::Logger* log_;
};


inline void escapeText( TmpBufBase<char>& buf,
                        const char* from, size_t len,
                        bool quoteAsCsv = false )
{
    TextEscaper te;
    te.escapeText(buf,from,len,quoteAsCsv);
}


inline char* unescapeText( char* text, const char* from = 0,
                           size_t len = 0, bool quoteAsCsv = false) {
    TextEscaper te;
    return te.unescapeText(text,from,len,quoteAsCsv);
}

} // informer
} // smsc

#endif
