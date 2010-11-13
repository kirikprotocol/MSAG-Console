#ifndef _INFORMER_RETRYPOLICY_H
#define _INFORMER_RETRYPOLICY_H

#include <vector>
#include "informer/io/Typedefs.h"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "RetryString.h"

namespace smsc {
namespace util {
namespace config {
class Config;
}
}
}

namespace eyeline {
namespace informer {

/// the instance of this class is attached to each center.
class RetryPolicy
{
public:
    RetryPolicy() {}
    ~RetryPolicy();

    void init( smsc::util::config::Config* cfg );

    /// calculate the retry interval for given delivery
    /// <0 - no retry -- permanent error
    ///  0 - immediate retry
    /// >0 - delayed retry
    timediff_type getRetryInterval( bool      isTransactional,
                                    int       smppState,
                                    uint16_t  retryCount ) const;

private:
    int nextItem( const char*& p,
                  const char*  policy,
                  const char*  items );

private:
    typedef smsc::core::buffers::IntHash< RetryString* > SmppHash;
    typedef std::vector< RetryString* >                  PolicyList;

    /// if the code is not found it is permanent.
    /// if the code is found, but the pointer is null it is immediate,
    /// otherwise -- use policy.
    mutable smsc::core::synchronization::Mutex lock_;
    SmppHash                           smppHash_;
    PolicyList                         policyList_;
};

} // informer
} // smsc

#endif
