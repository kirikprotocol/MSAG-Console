#ifndef _SCAG_COUNTER_IMPL_HASHCOUNTMANAGER_H
#define _SCAG_COUNTER_IMPL_HASHCOUNTMANAGER_H

#include <vector>
#include "scag/counter/Manager.h"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "TimeSliceManagerImpl.h"
#include "NotificationManager.h"

namespace scag2 {
namespace counter {
namespace impl {

// class NotificationManager;

class HashCountManager : public Manager, protected smsc::core::threads::Thread
{
public:
    /// NOTE: template manager gets owned
    HashCountManager( TemplateManager* tmplmgr = 0, unsigned notifySlices = 5 );
    virtual ~HashCountManager();

    void start();
    void stop();

    virtual TimeSliceManager& getTimeManager() {
        return *timeSliceManager_;
    }
    virtual TemplateManager* getTemplateManager() {
        return templateManager_;
    }

    virtual void notify( const char* cname, CntSeverity& sev,
                         int64_t value, const ActionLimit& params )
    {
        notificationManager_->notify(cname,sev,value,params);
    }

protected:
    /// retrieve the counter of type T with name 'name'
    /// it may return 0 if the counter is not there.
    virtual CounterPtrAny getAnyCounter( const char* name );

    /// register counter and return a ptr to it, or to existing counter of this name.
    /// it may throw exception if counter types are not the same.
    /// otherwise it always return a ptr to a good counter.
    /// NOTE: don't use 'c' pointer after the call, the object under it may be destroyed!
    virtual CounterPtrAny doRegisterAnyCounter( Counter* c, bool& wasRegistered );
    virtual void scheduleDisposal( Counter& c );
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
    // counttime_type                             nextTime_;

    typedef std::vector< Counter* >            DisposeQueueType;

    DisposeQueueType                           disposeQueue_;
    NotificationManager*                       notificationManager_;
    TimeSliceManagerImpl*                      timeSliceManager_;
    TemplateManager*                           templateManager_;
    bool stopping_;
};

}
}
}

#endif
