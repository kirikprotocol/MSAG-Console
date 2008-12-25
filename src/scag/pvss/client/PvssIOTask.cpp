#include "PvssIOTask.h"
#include "PvssConnection.h"
#include "PvssStreamClient.h"

namespace scag2 {
namespace pvss {
namespace client {

bool PvssIOTask::setupSockets()
{
    bool res = false;
    MutexGuard mg(mon_);
    if ( sockets_.Count() == 0 ) {
        mon_.wait(200);
        return false;
    }
    mul_.clear();
    for ( int i = 0; i < sockets_.Count(); ) {
        if ( sockets_[i]->isConnected() ) {
            res = setupSocket( * sockets_[i] );
            // mul_.add( sockets_[i]->socket() );
            // res = true;
            ++i;
        } else {
            PvssConnection* con = sockets_[i];
            sockets_.Delete(i);
            pers_->disconnected( *con );
        }
    }
    return res;
}


void PvssIOTask::processEvents()
{
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
