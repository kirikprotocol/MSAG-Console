#ifndef _SCAG_COUNTER_IMPL_HASHCOUNTMANAGER_H
#define _SCAG_COUNTER_IMPL_HASHCOUNTMANAGER_H

#include <vector>
#include "scag/counter/Manager.h"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"

namespace scag2 {
namespace counter {
namespace impl {

class HashCountManager : public Manager, protected smsc::core::threads::Thread
{
public:
    HashCountManager();
    virtual ~HashCountManager();

    virtual void start() {
        stopping_ = false;
        this->Start();
    }

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
    virtual counttime_type getWakeTime() const;
    inline counttime_type getCurrentTime() const {
        return counttime_type(time(NULL));
    }

    virtual int Execute();

private:
    smsc::logger::Logger*                      log_;

    smsc::core::synchronization::Mutex         hashMutex_;
    smsc::core::buffers::Hash< Counter* >      hash_;

    smsc::core::synchronization::EventMonitor  disposeMon_; // we are waiting on it

    counttime_type                             wakeTime_;
    counttime_type                             nextTime_;

    typedef std::vector< Counter* >            DisposeQueueType;
    DisposeQueueType                           disposeQueue_;

    bool stopping_;
};

}
}
}

#endif
