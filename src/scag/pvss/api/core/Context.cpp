#include "Context.h"
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"

namespace {
smsc::logger::Logger* logger = 0;
smsc::core::synchronization::Mutex mtx;
}

namespace scag2 {
namespace pvss {
namespace core {

Context::Context(Request* req) :
creationTime( util::currentTimeMillis() ),
request( req ) {
    if ( ! logger ) {
        MutexGuard mg(mtx);
        if ( ! logger ) logger = smsc::logger::Logger::getInstance("pvss.ctx");
    }
    assert(req != 0);
    smsc_log_debug(logger,"ctor: context %p: seqnum=%d",this,req->getSeqNum());
}


Context::~Context()
{
    smsc_log_debug(logger,"dtor: context %p: seqnum=%d",this,request.get() ? request->getSeqNum() : 0);
}

} // namespace core
} // namespace pvss
} // namespace scag2
