#include "IOTask.h"
#include "PvssSocket.h"
#include "Core.h"

namespace scag2 {
namespace pvss {
namespace core {

int IOTask::setupSockets(util::msectime_type currentTime)
{
    int ready = 0;
    closed_.clear();
    do {
        if ( workingSockets_.empty() ) break;

        mul_.clear();
        for ( size_t i = 0, ie = workingSockets_.size(); i < ie; ) {
            if ( workingSockets_[i]->isConnected() ) {
                if ( setupSocket(*workingSockets_[i], currentTime) ) ++ready;
                ++i;
            } else {
                // PvssSocket* con = sockets_[i];
                closed_.push_back(workingSockets_[i]);
                workingSockets_.erase( workingSockets_.begin()+i );
                --ie;
                // pers_->disconnected( *con );
            }
        }
    } while ( false );
    // smsc_log_debug( log_, "setup sockets: %d sockets ready", ready );
    return ready > 0 ? 0 : 300;
}

void IOTask::processEvents()
{
    smsc_log_debug( log_, "process sockets: error=%d ready=%d", error_.Count(), ready_.Count() );
    for ( int i = 0; i < error_.Count(); ++i ) {
        // smsc_log_warn( log_, "error on socket %p", error_[i] );
        core_->closeChannel( *PvssSocket::fromSocket(error_[i]) );
    }
    for ( int i = 0; i < ready_.Count(); ++i ) {
        process( *PvssSocket::fromSocket(ready_[i]) );
    }
}

void IOTask::postProcess()
{
    for ( ConnArray::iterator i = closed_.begin(), ie = closed_.end();
          i != ie; ++i ) {
        detachFromSocket(**i);
    }
    closed_.clear();
}

} // namespace core
} // namespace pvss
} // namespace scag2
