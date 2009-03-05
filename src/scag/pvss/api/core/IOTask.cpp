#include "IOTask.h"
#include "PvssSocket.h"
#include "Core.h"

namespace scag2 {
namespace pvss {
namespace core {

bool IOTask::setupSockets(util::msectime_type currenTime)
{
    int ready = 0;
    do {
        if ( sockets_.Count() == 0 ) break;

        mul_.clear();
        for ( int i = 0; i < sockets_.Count(); ) {
            if ( sockets_[i]->isConnected() ) {
                if ( setupSocket( * sockets_[i] ) ) ++ready;
                ++i;
            } else {
                // PvssSocket* con = sockets_[i];
                detachFromSocket(*sockets_[i]);
                sockets_.Delete(i);
                // pers_->disconnected( *con );
            }
        }
    } while ( false );
    // smsc_log_debug( log_, "setup sockets: %d sockets ready", res );
    return ready > 0;
}

void IOTask::processEvents()
{
    // smsc_log_debug( log_, "process sockets: error=%d ready=%d", error_.Count(), ready_.Count() );
    for ( int i = 0; i < error_.Count(); ++i ) {
        // smsc_log_warn( log_, "error on socket %p", error_[i] );
        core_->closeChannel( *PvssSocket::fromSocket(error_[i]) );
    }
    for ( int i = 0; i < ready_.Count(); ++i ) {
        process( *PvssSocket::fromSocket(ready_[i]) );
    }
}

} // namespace core
} // namespace pvss
} // namespace scag2
