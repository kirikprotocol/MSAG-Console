#include <string.h>
#include "RetryString.h"
#include "informer/io/InfosmeException.h"

namespace eyeline {
namespace informer {

RetryString::RetryString() :
items_(0), itemsize_(0)
{
}


RetryString::~RetryString() {
    delete [] items_;
}


RetryString::RetryString( const RetryString& rs ) :
items_(0), itemsize_(rs.itemsize_)
{
    if (itemsize_) {
        items_ = new RetryItem[itemsize_];
        for ( unsigned i = 0; i < itemsize_; ++i ) {
            items_[i] = rs.items_[i];
        }
    }
}


RetryString& RetryString::operator = ( const RetryString& rs )
{
    if (&rs != this) {
        delete [] items_;
        itemsize_ = rs.itemsize_;
        if ( itemsize_ ) {
            items_ = new RetryItem[itemsize_];
            for ( unsigned i = 0; i < itemsize_; ++i ) {
                items_[i] = rs.items_[i];
            }
        } else {
            items_ = 0;
        }
    }
    return *this;
}


void RetryString::init( const char* line )
{
    if (line) {
        while (*line == ' ') {
            ++line;
        }
    }
    delete [] items_;
    items_ = 0;
    itemsize_ = 0;
    if (!line || line[0] == '\0') { return; }

    // calculating the number of elements in items
    itemsize_ = 0;
    for ( const char* p = line; (p = strchr(p,',')) != NULL; ++itemsize_ ) {
        ++p;
    }
    ++itemsize_;
    items_ = new RetryItem[itemsize_];
    try {
        unsigned idx = 0;
        bool lastfound = false;
        unsigned attempts = 0;
        for ( const char* p = line; ; ) {

            timediff_type res = 0;
            unsigned repetition = 1;
            int shift = 0;
            char type;
            unsigned val;
            if ( -1 == sscanf(p,"%u%c%n",&val,&type,&shift) ) {
                throw InfosmeException(EXC_IOERROR,"invalid retry string '%s' at pos=%u",line,unsigned(p-line));
            }
            if (shift > 0) {
                switch (type) {
                case 'D':
                case 'd': res += val*24*60*60; break;
                case 'H':
                case 'h': res += val*60*60; break;
                case 'M':
                case 'm': res += val*60; break;
                case 'S':
                case 's': res += val; break;
                default :
                    throw InfosmeException(EXC_IOERROR,"invalid suffix '%c' in retry string '%s' at pos %u",
                                           type,line,unsigned(p-line));
                }
                p += shift;
            }

            while (*p == ' ') { ++p; }

            if (*p == ':') {
                // repetition count
                if (*++p == '*') {
                    // last item found
                    repetition = 0xffff - attempts;
                    lastfound = true;
                    ++p;
                } else {
                    shift = 0;
                    if ( -1 == sscanf(p,"%u%n",&repetition,&shift) || shift == 0 ) {
                        throw InfosmeException(EXC_IOERROR,"invalid repetition count in '%s' at pos %u",
                                               line, unsigned(p-line));
                    }
                    p += shift;
                }
            }

            while (*p == ' ') { ++p; }

            if ( *p == '\0' || *p == ',' ) {
                // new items ends here
                if (repetition == 0) {
                    throw InfosmeException(EXC_IOERROR,"repetition == 0 in '%s' before %u",
                                           line,unsigned(p-line));
                }
                if (repetition > 0xffff) {
                    throw InfosmeException(EXC_IOERROR,"too big repetition in '%s' before %u",
                                           line, unsigned(p-line));
                }
                attempts += repetition;
                if (attempts > 0xffff) {
                    throw InfosmeException(EXC_IOERROR,"too many attemts accumulated in '%s' before %u",
                                           line, unsigned(p-line));
                }
                items_[idx].count = attempts;
                items_[idx].interval = res;
                ++idx;
                if ( (*p == '\0' && idx != itemsize_) ||
                     (*p != '\0' && idx >= itemsize_) ) {
                    throw InfosmeException(EXC_LOGICERROR,"miscounted items in '%s' (idx=%u itemsize=%u eol=%d), mea culpa",
                                           line, idx, itemsize_, *p == '\0' );
                }
                if (*p == '\0') {
                    break;
                }
                ++p;
            } else {
                throw InfosmeException(EXC_IOERROR,"invalid char '%c' in '%s' at pos %u",
                                       *p, line, unsigned(p-line));
            }
        }

    } catch ( std::exception& e ) {
        delete [] items_;
        items_ = 0;
        itemsize_ = 0;
        throw;
    }
}


timediff_type RetryString::getRetryInterval( uint16_t retryCount ) const
{
    if (!items_) return -1; // no retries
    for ( unsigned i = 0; i < itemsize_; ++i ) {
        if ( retryCount < items_[i].count ) {
            return items_[i].interval;
        }
    }
    return -1;
}

} // informer
} // smsc
