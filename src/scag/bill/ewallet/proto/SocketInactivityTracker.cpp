#include "SocketInactivityTracker.h"
#include "Core.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

bool SocketInactivityTracker::setupSockets( util::msectime_type currentTime )
{
    stalledSockets_.Empty();
    wakeupTime_ = currentTime + core_.getConfig().getInactivityTimeout();
    for ( int i = 0; i < sockets_.Count(); ++i ) {
        Socket& socket = *sockets_[i];
        if ( socket.isConnected() ) {
            const util::msectime_type limit = 
                socket.getLastActivity() + core_.getConfig().getInactivityTimeout();
            if ( limit < currentTime ) {
                // socket is stalled
                stalledSockets_.Push( &socket );
            } else if ( limit < wakeupTime_ ) {
                wakeupTime_ = limit;
            }
        }
    }
    return stalledSockets_.Count() > 0;
}


bool SocketInactivityTracker::hasEvents()
{
    return stalledSockets_.Count() > 0;
}


void SocketInactivityTracker::processEvents()
{
    for ( int i = 0; i < stalledSockets_.Count(); ++i ) {
        core_.inactivityTimeout( *stalledSockets_[i] );
    }
}


} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2
