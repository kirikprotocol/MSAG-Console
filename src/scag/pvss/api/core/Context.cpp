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
creationTime_( util::currentTimeMillis() ),
request_( req ) {
    if ( ! log_ ) {
        MutexGuard mg(mtx);
        if ( ! log_ ) log_ = smsc::logger::Logger::getInstance("pvss.ctx");
    }
    assert(req != 0);
    smsc_log_debug(log_,"ctor: context %p: seqnum=%d",this,req->getSeqNum());
}


Context::~Context()
{
    smsc_log_debug(log_,"dtor: context %p: seqnum=%d",this,request_.get() ? request_->getSeqNum() : 0);
}


void Context::setResponse( Response* resp ) throw (PvssException)
{
    smsc_log_debug(log_,"context %p setting resp %p: %s",this,resp, resp ? resp->toString().c_str() : "");
    response_.reset(resp);
}

} // namespace core
} // namespace pvss
} // namespace scag2
