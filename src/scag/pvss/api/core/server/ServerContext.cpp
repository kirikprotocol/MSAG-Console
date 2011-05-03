#include "ServerContext.h"
#include "ServerCore.h"
#include "scag/pvss/api/packets/ErrorResponse.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

void ServerContext::setResponse(Response* response) /* throw (PvssException) */ 
{
    smsc::core::synchronization::MutexGuard mg(setRespMutex_);
    if ( state_ == NEW && response != 0 ) {
        state_ = PROCESSED;
        response->setSeqNum(getSeqNum());
        Context::setResponse(response);
        // core_.contextProcessed(*this);
    } else {
        throw PvssException(PvssException::BAD_RESPONSE,"Response is null or was already processed");
    }
}


void ServerContext::setError( const std::string& msg, Response::Type type ) /* throw(PvssException) */ 
{
    setResponse( new ErrorResponse( getSeqNum(), type, msg.empty() ? 0 : msg.c_str()));
}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
