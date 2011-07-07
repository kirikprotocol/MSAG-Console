#include "PvssIOTask.h"
#include "PvssConnection.h"
#include "PvssStreamClient.h"

using namespace smsc::core::synchronization;

namespace scag2 {
namespace pvss {
namespace client {

bool PvssIOTask::setupSockets()
{
    int res = 0;
    MutexGuard mg(mon_);
    do {
        if ( sockets_.Count() == 0 ) {
            mon_.wait(200);
            break;
        }
        mul_.clear();
        for ( int i = 0; i < sockets_.Count(); ) {
            if ( sockets_[i]->isConnected() ) {
                if ( setupSocket( * sockets_[i] ) ) ++res;
                // mul_.add( sockets_[i]->socket() );
                // res = true;
                ++i;
            } else {
                PvssConnection* con = sockets_[i];
                sockets_.Delete(i);
                pers_->disconnected( *con );
            }
        }
    } while ( false );
    // smsc_log_debug( log_, "setup sockets: %d sockets ready", res );
    return res > 0;
}


void PvssIOTask::processEvents()
{
    // smsc_log_debug( log_, "process sockets: error=%d ready=%d", error_.Count(), ready_.Count() );
    for ( int i = 0; i < error_.Count(); ++i ) {
        smsc_log_warn( log_, "error on socket %p", error_[i] );
        PvssConnection::fromSocket(error_[i])->disconnect();
    }
    for ( int i = 0; i < ready_.Count(); ++i ) {
        process( * PvssConnection::fromSocket(ready_[i]) );
    }
}

} // namespace client
} // namespace pvss
} // namespace scag2
