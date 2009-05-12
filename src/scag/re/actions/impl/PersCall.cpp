#include "PersCall.h"
#include "scag/lcm/base/LongCallManager2.h"

using namespace scag2::pvss;


namespace {
bool loggerInited = false;
smsc::core::synchronization::Mutex logmutex;
smsc::logger::Logger* thelogger;
smsc::logger::Logger* logger()
{
    if ( ! loggerInited ) {
        smsc::core::synchronization::MutexGuard mg(logmutex);
        if ( ! loggerInited ) {
            thelogger = smsc::logger::Logger::getInstance("perscall");
        }
    }
    return thelogger;
}

}

namespace scag2 {
namespace re {
namespace actions {

PersCall::PersCall( ProfileRequest* req ) :
exc_(std::string(),PvssException::OK), req_(req) {}

void PersCall::handleResponse( std::auto_ptr< Request > request, std::auto_ptr< Response > response )
{
    assert( lcc_ );
    req_.reset( static_cast< ProfileRequest* >(request.release()) );
    resp_.reset( static_cast< ProfileResponse* >(response.release()));
    // continue lcm execution
    lcc_->initiator->continueExecution( lcc_, false );
    // NOTE: we clear req and resp here in controlled manner
    req_.reset(0);
    resp_.reset(0);
}

void PersCall::handleError( const pvss::PvssException& exc, std::auto_ptr< pvss::Request > request )
{
    assert( lcc_ );
    setError( exc, request );
    // continue lcm execution
    lcc_->initiator->continueExecution( lcc_, false );
    req_.reset(0);
}


void PersCall::setError( const pvss::PvssException& exc, std::auto_ptr< pvss::Request > request )
{
    req_.reset( static_cast< ProfileRequest* >(request.release()));
    exc_ = exc;
}


bool PersCall::doCallPvss()
{
    std::auto_ptr< Request > request(req_.release());
    try {

        pvss::core::client::Client& clnt = lcm::LongCallManager::Instance().pvssClient();
        clnt.processRequestAsync( request, *this );
        
    } catch ( PvssException& e ) {
        smsc_log_warn( logger(), "cannot call pvss: %s %s",
                       e.statusToString(e.getType()), e.what());
        setError( e, request );
        return false;
    } catch ( std::exception& e ) {
        smsc_log_warn( logger(), "cannot call pvss: %s", e.what());
        setError( PvssException(e.what(),PvssException::UNKNOWN), request );
        return false;
    }
    return true;
}


} // namespace actions
} // namespace re
} // namespace scag2
