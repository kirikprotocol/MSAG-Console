#include "FlooderThread.h"
#include "FlooderStat.h"
#include "RequestGenerator.h"
#include "scag/pvss/api/packets/ProfileRequest.h"

namespace scag2 {
namespace pvss {
namespace flooder {

FlooderThread::FlooderThread( FlooderStat& flooderStat, bool oneCommandPerAbonent ) :
log_(smsc::logger::Logger::getInstance(taskName())),
stat_(flooderStat),
pattern_(0),
profileKey_(stat_.getGenerator().getProfileKey()),
oneCommandPerAbonent_(oneCommandPerAbonent)
{
    assert(stat_.getGenerator().getPatterns().size() > 0 );
}


std::auto_ptr<Request> FlooderThread::generate()
{
    while ( true ) {
        ProfileCommand* cmd = stat_.getGenerator().generateCommand(pattern_);
        if ( ! cmd || oneCommandPerAbonent_ ) {
            // switching to a new abonent
            profileKey_ = stat_.getGenerator().getProfileKey();
        }
        if ( ! cmd ) continue;
        return std::auto_ptr<Request>(new ProfileRequest(profileKey_,cmd));
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
    // request->timingMark("bProcAsync");
    stat_.getClient().processRequestAsync(request,stat_);
}


void SyncFlooderThread::doProcessRequest( std::auto_ptr< Request >& request ) /* throw (PvssException) */ 
{
    // request->timingMark("bProcSync");
    std::auto_ptr<Response> response = stat_.getClient().processRequestSync(request);
    // request->timingMark("aProcSync");
    stat_.handleResponse(request,response);
}


} // namespace flooder
} // namespace pvss
} // namespace scag2
