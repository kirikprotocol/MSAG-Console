#ifndef _SCAG_PVSS_CORE_SERVER_WORKER_H
#define _SCAG_PVSS_CORE_SERVER_WORKER_H

#include "Server.h"
#include "ContextQueue.h"
#include "scag/util/WatchedThreadedTask.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class ServerConfig;
class ServerCore;

class Worker : public util::WatchedThreadedTask
{
public:
    Worker( Server::SyncLogic& logic, ServerCore& core ) : logic_(logic), core_(core) {}

    /// return attached queue
    inline ContextQueue& getQueue() { return queue_; }

    virtual ~Worker();

    virtual int Execute();

    /// get the config
    ServerConfig& getConfig();

private:
    Server::SyncLogic& logic_;
    ContextQueue       queue_;
    ServerCore&        core_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_WORKER_H */
