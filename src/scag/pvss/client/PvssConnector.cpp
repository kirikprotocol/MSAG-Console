#include "PvssConnector.h"
#include "PvssConnection.h"
#include "PvssStreamClient.h"

using namespace smsc::core::synchronization;

namespace scag2 {
namespace pvss {
namespace client {

bool PvssConnector::setupSockets()
{
    MutexGuard mg(mon_);
    removeConnected();
    if ( sockets_.Count() == 0 ) {
        mon_.wait(200);
        return false;
    }
    worksock_ = sockets_;
    // waiting until nearest connect time
    time_t mintime = worksock_[0]->nextConnectTime();
    for ( int i = 0; i < worksock_.Count(); ++i ) {
        const time_t conntime = worksock_[i]->nextConnectTime();
        if ( conntime < mintime ) {
            mintime = conntime;
        }
    }
    const time_t now = time(0);
    if ( mintime > now ) {
        mon_.wait((mintime-now)*1000);
    }
    return true;
}


bool PvssConnector::hasEvents()
{
    return ( worksock_.Count() > 0 );
}


void PvssConnector::processEvents()
{
    for ( int i = 0; i < worksock_.Count(); ++i ) {
        worksock_[i]->connect();
    }
    {
        MutexGuard mg(mon_);
        removeConnected();
    }
    pers_->checkConnections();
}


void PvssConnector::removeConnected()
{
    for ( int i = 0; i < sockets_.Count(); ) {
        if ( sockets_[i]->isConnected() ) {
            PvssConnection* con = sockets_[i];
            sockets_.Delete(i);
            pers_->connected( *con );
        } else {
            // the socket could not connect
            sockets_[i]->dropCalls();
            ++i;
        }
    }
}

} // namespace client
} // namespace pvss
} // namespace scag2
