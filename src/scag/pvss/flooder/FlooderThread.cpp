#include "FlooderThread.h"
#include "FlooderStat.h"
#include "RequestGenerator.h"
#include "scag/pvss/api/packets/AbstractProfileRequest.h"

namespace scag2 {
namespace pvss {
namespace flooder {

FlooderThread::FlooderThread( FlooderStat&          flooderStat ) :
log_(smsc::logger::Logger::getInstance(taskName())),
stat_(flooderStat),
pattern_(0),
profileKey_(stat_.getGenerator().getProfileKey())
{
    assert(stat_.getGenerator().getPatterns().size() > 0 );
}


std::auto_ptr<Request> FlooderThread::generate()
{
    while ( true ) {
        AbstractCommand* cmd = stat_.getGenerator().generateCommand(pattern_);
        if ( ! cmd ) {
            // end of command patterns reached, we have to switch to a new profile key
            profileKey_ = stat_.getGenerator().getProfileKey();
            continue;
        }
        AbstractProfileRequest* req = AbstractProfileRequest::create(cmd);
        req->setProfileKey( profileKey_ );
        // smsc_log_debug(log_,"request %s created", req->toString().c_str());
        return std::auto_ptr<Request>(req);
    }
}


int FlooderThread::doExecute()
{
    while ( ! isStopping ) {
        if ( stat_.isStopped() ) break;
        std::auto_ptr< Request > request = generate();
        if ( ! request.get() ) continue;
        stat_.requestCreated( request.get() );
        try {
            // stat_.getClient().processRequestAsync(request,stat_);
            doProcessRequest(request);
        } catch ( PvssException& e ) {
            stat_.handleError(e,request);
        }
        stat_.adjustSpeed();
    }
    return 0;
}


void AsyncFlooderThread::doProcessRequest( std::auto_ptr< Request >& request ) /* throw (PvssException) */ 
{
    stat_.getClient().processRequestAsync(request,stat_);
}


void SyncFlooderThread::doProcessRequest( std::auto_ptr< Request >& request ) /* throw (PvssException) */ 
{
    std::auto_ptr<Response> response = stat_.getClient().processRequestSync(request);
    stat_.handleResponse(request,response);
}


} // namespace flooder
} // namespace pvss
} // namespace scag2
