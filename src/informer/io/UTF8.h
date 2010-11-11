#ifndef _INFORMER_UTF8_H
#define _INFORMER_UTF8_H

#include <iconv.h>
#include "core/buffers/TmpBuf.hpp"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

class UTF8
{
public:
    typedef smsc::core::buffers::TmpBuf<char,2048> BufType;

    UTF8();
    ~UTF8();

    /// convert input utf8 message into ucs2.
    void convertToUcs2( const char* in, size_t inlen, BufType& buf );

private:
    smsc::core::synchronization::Mutex lock_;
    iconv_t                            conv_;
};

} // informer
} // eyeline

#endif
