#ifndef _SCAG_PVSS_CORE_SERVER_WORKER_H
#define _SCAG_PVSS_CORE_SERVER_WORKER_H

#include "Server.h"
#include "ContextQueue.h"
#include "scag/util/WatchedThreadedTask.h"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class ServerConfig;
class ServerCore;

class Worker : public util::WatchedThreadedTask
{
public:
    Worker( Server::SyncLogic& logic, ServerCore& core );

    virtual const char* taskName() { return "pvss.workr"; }

    /// return attached queue
    inline ContextQueue& getQueue() { return queue_; }
    inline ContextQueue& getRespQueue() { return respQueue_; }

    virtual ~Worker();

    void shutdown();

    virtual int Execute();

    /// get the config
    ServerConfig& getConfig();

private:
    Server::SyncLogic& logic_;
    ContextQueue       queue_;
    ContextQueue       respQueue_;
    ServerCore&        core_;
    smsc::logger::Logger* log_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_WORKER_H */
