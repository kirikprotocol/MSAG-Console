#ifndef _SCAG_PVSS_CLIENT_PVSSCONNTASK_H
#define _SCAG_PVSS_CLIENT_PVSSCONNTASK_H

#include "core/threads/ThreadedTask.hpp"
#include "core/network/Socket.hpp"
#include "core/buffers/Array.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {
namespace client {

class PvssConnection;
class PvssStreamClient;

class PvssConnTask : public smsc::core::threads::ThreadedTask
{
public:
    PvssConnTask( PvssStreamClient& pers,
                  smsc::logger::Logger* logger ) : pers_(&pers), log_(logger) {}

    void addConnection( PvssConnection& con );
    virtual int Execute();

protected:
    /// setup things prior the invocation of hasEvents()
    virtual bool setupSockets() = 0;

    /// action taken on setup sockets failure
    virtual void setupFailed() {}

    /// check if there are some events
    virtual bool hasEvents() = 0;

    /// process events
    virtual void processEvents() = 0;

    /// process no events
    virtual void processNoEvents() {}

protected:
    smsc::core::synchronization::EventMonitor     mon_;
    smsc::core::buffers::Array< PvssConnection* > sockets_;
    PvssStreamClient*                             pers_;
    smsc::logger::Logger*                         log_;
};

} // namespace client
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CLIENT_PVSSCONNTASK_H */
