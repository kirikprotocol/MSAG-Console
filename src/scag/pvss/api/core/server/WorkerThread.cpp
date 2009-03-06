#include "WorkerThread.h"
#include "ServerConfig.h"
#include "ServerCore.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

WorkerThread::WorkerThread( ServerCore& core, const char* cat ) :
queue_(core.getConfig().getQueueSizeLimit()),
core_(core),
log_(smsc::logger::Logger::getInstance(cat))
{}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
