#ifndef _SCAG_PVSS_CORE_SOCKTASK_H
#define _SCAG_PVSS_CORE_SOCKTASK_H

#include <vector>
#include "scag/util/WatchedThreadedTask.h"
#include "core/network/Socket.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"
#include "scag/exc/IOException.h"
#include "scag/util/Time.h"
#include "PvssSocket.h"

namespace scag2 {
namespace pvss {
namespace core {

class Config;
class Core;

class SockTask : public util::WatchedThreadedTask
{
protected:
    SockTask( Config& theconfig,
              Core& thecore,
              const char* logname = 0 ) :
    config_(&theconfig), core_(&thecore), taskname_(logname ? logname : "pvss.task"),
    log_(0) {
        log_ = smsc::logger::Logger::getInstance(taskname_.c_str());
    }

public:
    virtual const char* taskName() { return taskname_.c_str(); }

    virtual ~SockTask() {
        smsc_log_info(log_,"dtor: socktask %s",taskName());
    }

    virtual void shutdown() {
        smsc::core::synchronization::MutexGuard mg(mon_);
        if ( isStopping ) return;
        smsc_log_info(log_,"shutting down...");
        stop();
        mon_.notify();
        // wakeup();
    }

    inline unsigned sockets() const {
        return unsigned(workingSockets_.size()) + unsigned(pendingSockets_.size());
    }

    /// register a socket for this task ops
    void registerChannel( PvssSocketPtr& socket );

    /// unregister a socket for this task ops
    void unregisterChannel( PvssSocket& socket );

    // waking up
    virtual void wakeup() {
        smsc::core::synchronization::MutexGuard mg(mon_);
        mon_.notify();
    }

    virtual const Config& getConfig() const { return *config_; }

protected:
    virtual int doExecute();

    virtual void attachToSocket( PvssSocket& ) {}
    virtual void detachFromSocket( PvssSocket& ) {}

    /// setup things prior the invocation of hasEvents()
    /// @return the next wakeup time
    virtual int setupSockets(util::msectime_type currentTime) = 0;

    /// action taken on setup sockets failure
    virtual int setupFailed(int tmo) { return tmo; }

    /// check if there are some events
    virtual bool hasEvents() = 0;

    /// process events
    virtual void processEvents() = 0;

    /// post processing
    virtual void postProcess() {}

    typedef std::vector< PvssSocketPtr >  ConnArray;

private:
    void processPending();

protected:
    const Config*                                 config_;
    Core*                                         core_;

private:
    smsc::core::synchronization::EventMonitor     mon_;
    ConnArray                                     pendingSockets_;
    ConnArray                                     unpendingSockets_;

protected:
    // working array has no access from other threads
    ConnArray                                     workingSockets_;
    std::string                                   taskname_;
    smsc::logger::Logger*                         log_;
    // util::msectime_type                           wakeupTime_;
};

} // namespace core
} // namespace pvss
} // namespace scag2

namespace scag {
namespace pvss {
namespace core {

using scag2::pvss::core::SockTask;

}
}
}

#endif /* !_SCAG_PVSS_CORE_SOCKTASK_H */
