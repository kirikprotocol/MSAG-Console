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
        AbstractProfileRequest* req = stat_.getGenerator().generate(pattern_,&profileKey_);
        if ( req ) {
            smsc_log_debug(log_,"request %s created", req->toString().c_str());
            return std::auto_ptr<Request>(req);
        }
        profileKey_ = stat_.getGenerator().getProfileKey();
    }
}


int AsyncFlooderThread::Execute()
{
    smsc::core::synchronization::EventMonitor em;
    MutexGuard mg(em);
    while ( ! isStopping ) {
        if ( stat_.isStopped() ) break;
        std::auto_ptr< Request > request = generate();
        if ( ! request.get() ) continue;
        stat_.requestCreated();
        try {
            stat_.getClient().processRequestAsync(request,stat_);
        } catch ( PvssException& e ) {
            stat_.handleError(e,request);
        }
        stat_.adjustSpeed();
    }
    return 0;
}


int SyncFlooderThread::Execute()
{
    while ( ! isStopping ) {
        if ( stat_.isStopped() ) break;
        std::auto_ptr< Request > request = generate();
        if ( ! request.get() ) continue;
        stat_.requestCreated();
        try {
            std::auto_ptr<Response> response = stat_.getClient().processRequestSync(request);
            smsc_log_debug(log_,"response %s got", response->toString().c_str());
            stat_.handleResponse(request,response);
        } catch ( PvssException& e ) {
            stat_.handleError(e,request);
        }
        smsc_log_debug(log_,"adjusting speed");
        stat_.adjustSpeed();
    }
    return 0;
}


} // namespace flooder
} // namespace pvss
} // namespace scag2
