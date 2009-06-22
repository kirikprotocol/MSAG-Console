#include "IOSocketTask.h"
#include "Core.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

bool IOSocketTask::setupSockets( util::msectime_type currentTime )
{
    int ready = 0;
    closed_.Empty();
    do {
        if ( sockets_.Count() == 0 ) break;

        mul_.clear();
        for ( int i = 0; i < sockets_.Count(); ) {
            if ( sockets_[i]->isConnected() ) {
                if ( setupSocket(*sockets_[i], currentTime) ) ++ready;
                ++i;
            } else {
                closed_.Push(sockets_[i]->socket());
                sockets_.Delete(i);
            }
        }
    } while ( false );
    // smsc_log_debug( log_, "setup sockets: %d sockets ready", res );
    return ready > 0;
}


void IOSocketTask::processEvents()
{
    for ( int i = 0; i < error_.Count(); ++i ) {
        // smsc_log_warn( log_, "error on socket %p", error_[i] );
        Socket* socket = Socket::fromSocket(*error_[i]);
        core_.closeSocket( *socket );
    }
    for ( int i = 0; i < ready_.Count(); ++i ) {
        process( *Socket::fromSocket(*ready_[i]) );
    }
}


void IOSocketTask::postProcess()
{
    for ( int i = 0; i < closed_.Count(); ++i ) {
        Socket* socket = Socket::fromSocket(*closed_[i]);
        detachSocket(*socket);
    }
}

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2
