#include "AsyncDispatcherThread.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

AsyncDispatcherThread::AsyncDispatcherThread( ServerCore& core, AsyncDispatcher& dispatcher ) :
WorkerThread(core,taskName()),
dispatcher_(&dispatcher)
{
}


AsyncDispatcherThread::~AsyncDispatcherThread()
{
    shutdown();
}


int AsyncDispatcherThread::Execute()
{
    // FIXME
    return 0;
}


void AsyncDispatcherThread::init() throw (PvssException)
{
    // FIXME
}


void AsyncDispatcherThread::shutdown()
{
    queue_.shutdown();
    // FIXME
}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
