#ifndef _INFORMER_RETRYSTRING_H
#define _INFORMER_RETRYSTRING_H

#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

/// this class determines the retry interval based on the current retry attempt.
class RetryString
{
    struct RetryItem {
        uint16_t       count;
        timediff_type  interval;
    };

public:
    RetryString();
    ~RetryString();
    RetryString( const RetryString& );
    RetryString& operator = ( const RetryString& );

    void init( const char* line );

    /// calculate the next retry interval (seconds) based on current retry count.
    /// @param retryCount -- the number of retries made (starting from 0).
    /// @return the retry time:
    /// <0 -- no retry
    ///  0 -- immediate retry
    /// >0 -- actual retry time
    timediff_type getRetryInterval( uint16_t retryCount ) const;

private:
    RetryItem* items_;
    unsigned   itemsize_;
};

} // informer
} // smsc

#endif
