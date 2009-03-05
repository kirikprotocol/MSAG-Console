#include "Request.h"
#include "Response.h"
#include "ResponseTypeMatch.h"

namespace {
scag2::pvss::ResponseTypeMatch responseMatch_;
smsc::core::synchronization::Mutex logMtx;
}

namespace scag2 {
namespace pvss {

smsc::logger::Logger* Request::log_ = 0;

Request::Request() : Packet() 
{
    if (!log_) {
        MutexGuard mg(logMtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("pvss.req");
    }
    smsc_log_debug(log_,"ctor: %p",this);
}

Request::~Request()
{
    smsc_log_debug(log_,"dtor: %p",this);
}

bool Request::matchResponseType( const Response& resp ) const
{
    try {
        return const_cast< Response& >(resp).visit( getResponseTypeMatch() );
    } catch ( PvapException& e ) {
        return false;
    }
}

ResponseTypeMatch& Request::getResponseTypeMatch() const {
    return responseMatch_;
}

} // namespace pvss
} // namespace scag2
