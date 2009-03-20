#ifndef _SCAG_PVSS_CORE_SERVER_WORKER_H
#define _SCAG_PVSS_CORE_SERVER_WORKER_H

#include "WorkerThread.h"
#include "logger/Logger.h"
#include "Server.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class Worker : public WorkerThread
{
public:
    Worker( Server::SyncLogic& logic, ServerCore& core );

    virtual const char* taskName() { return "pvss.workr"; }

    virtual ~Worker();

    void shutdown();

private:
    virtual int doExecute();

private:
    Server::SyncLogic& logic_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_WORKER_H */
