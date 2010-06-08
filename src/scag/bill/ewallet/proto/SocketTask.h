#ifndef SCAG_BILL_EWALLET_PROTO_SOCKETTASK_H
#define SCAG_BILL_EWALLET_PROTO_SOCKETTASK_H

#include "scag/util/WatchedThreadedTask.h"
#include "core/buffers/Array.hpp"
#include "Socket.h"
#include "logger/Logger.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

class Core;

class SocketTask : public util::WatchedThreadedTask
{
public:
    SocketTask( Core& core, const char* taskname ) :
    core_(core), taskname_(taskname), log_(0), wakeupTime_(0) {
        log_ = smsc::logger::Logger::getInstance(taskname_.c_str());
    }

    virtual const char* taskName() { return taskname_.c_str(); }

    virtual ~SocketTask() {
        smsc_log_info(log_,"dtor: SocketTask %s", taskname_.c_str());
    }

    virtual void shutdown();

    virtual void registerSocket( Socket& socket );

    virtual void unregisterSocket( Socket& socket );

    virtual void wakeup();

    size_t sockets() const;

protected:
    /// main loop
    virtual int doExecute();

    /// NOTE: mon is locked
    virtual bool setupSockets( util::msectime_type currentTime ) = 0;
    /// NOTE: mon is locked
    virtual void setupFailed( util::msectime_type currentTime );

    virtual bool hasEvents() = 0;
    virtual void processEvents() = 0;
    virtual void postProcess() {}

    virtual void attachSocket( Socket& socket );
    virtual void detachSocket( Socket& socket );

protected:
    Core&       core_;
    std::string taskname_;

    mutable smsc::core::synchronization::EventMonitor mon_;
    smsc::core::buffers::Array< Socket* >     sockets_;
    smsc::logger::Logger*                     log_;
    util::msectime_type                       wakeupTime_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_SOCKETTASK_H */
