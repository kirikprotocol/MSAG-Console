#ifndef _SCAG_COUNTER_IMPL_HASHCOUNTMANAGER_H
#define _SCAG_COUNTER_IMPL_HASHCOUNTMANAGER_H

#include <vector>
#include "scag/counter/Manager.h"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"

namespace scag2 {
namespace counter {
namespace impl {

class HashCountManager : public Manager
{
public:
    virtual ~HashCountManager();

    /// retrieve the counter of type T with name 'name'
    /// it may return 0 if the counter is not there.
    virtual CounterPtrAny getAnyCounter( const char* name );

protected:
    /// register counter and return a ptr to it, or to existing counter of this name.
    /// it may throw exception if counter types are not the same.
    /// otherwise it always return a ptr to a good counter.
    /// NOTE: don't use 'c' pointer after the call, the object under it may be destroyed!
    virtual CounterPtrAny registerAnyCounter( Counter* c );
    virtual void scheduleDisposal( counttime_type dt, Counter& c );
    virtual counttime_type getCurrentTime() const;

private:
    smsc::core::synchronization::Mutex         hashMutex_;
    smsc::core::buffers::Hash< Counter* > hash_;

    smsc::core::synchronization::EventMonitor  disposeMon_;
    counttime_type                             nextTime_;
    std::vector< Counter* >                    disposeQueue_;
};

}
}
}

#endif
