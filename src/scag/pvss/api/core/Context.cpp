#include "Context.h"
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"

namespace {
smsc::core::synchronization::Mutex mtx;
}

namespace scag2 {
namespace pvss {
namespace core {

smsc::logger::Logger* Context::log_ = 0;

Context::Context(Request* req) :
creationTime( util::currentTimeMillis() ),
request( req ) {
    if ( ! log_ ) {
        MutexGuard mg(mtx);
        if ( ! log_ ) log_ = smsc::logger::Logger::getInstance("pvss.ctx");
    }
    assert(req != 0);
    smsc_log_debug(log_,"ctor: context %p: seqnum=%d",this,req->getSeqNum());
}


Context::~Context()
{
    smsc_log_debug(log_,"dtor: context %p: seqnum=%d",this,request.get() ? request->getSeqNum() : 0);
}


void Context::setResponse( Response* resp ) throw (PvssException)
{
    smsc_log_debug(log_,"setting resp %p",resp);
    response.reset(resp);
}

} // namespace core
} // namespace pvss
} // namespace scag2
