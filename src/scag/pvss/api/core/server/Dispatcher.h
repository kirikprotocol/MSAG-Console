#ifndef _SCAG_PVSS_CORE_SERVER_DISPATCHER_H
#define _SCAG_PVSS_CORE_SERVER_DISPATCHER_H

#include "ContextQueue.h"
#include "scag/util/WatchedThreadedTask.h"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/core/server/Server.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

/// interface
class SyncDispatcher
{
public:
    virtual ~SyncDispatcher() {}
    virtual unsigned getIndex(Request& request) = 0;
    virtual Server::SyncLogic* getSyncLogic(unsigned index) = 0;
};

class AsyncDispatcher
{
public:
    virtual ~AsyncDispatcher() {}
    virtual unsigned getIndex(Request& request) = 0;
    virtual Server::AsyncLogic* getAsyncLogic(unsigned index) = 0;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_DISPATCHER_H */
