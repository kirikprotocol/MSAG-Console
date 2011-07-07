#include "PvssReader.h"
#include "PvssStreamClient.h"

namespace scag2 {
namespace pvss {
namespace client {

bool PvssReader::setupSocket( PvssConnection& con )
{
    mul_.add( con.socket() );
    return true;
}


void PvssReader::postProcess()
{
    const util::MsecTime::time_type now = time0_.msectime();
    if ( now - (lastCheck_ + pers_->timeout) <= 0 ) return;
    lastCheck_ = now;
    // cleanup sockets w/o events
    smsc::core::synchronization::MutexGuard mg(mon_);
    for ( int i = 0; i < sockets_.Count(); ++i ) {
        PvssConnection* con = sockets_[i];
        if ( con->isConnected() ) con->dropExpiredCalls();
    }
}

} // namespace client
} // namespace pvss
} // namespace scag2
