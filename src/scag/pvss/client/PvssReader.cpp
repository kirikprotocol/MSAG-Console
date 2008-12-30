#include "PvssReader.h"

namespace scag2 {
namespace pvss {
namespace client {

bool PvssReader::setupSocket( PvssConnection& con )
{
    con.setReady( false );
    mul_.add( con.socket() );
    return true;
}


void PvssReader::processEvents()
{
    PvssIOTask::processEvents();
    // cleanup sockets w/o events
    for ( int i = 0; i < sockets_.Count(); ++i ) {
        PvssConnection* con = sockets_[i];
        if ( con->isConnected() && ! con->isReady() ) {
            // no activity on this socket
            con->dropExpiredCalls();
        }
    }
}


void PvssReader::processNoEvents()
{
    for ( int i = 0; i < sockets_.Count(); ++i ) {
        PvssConnection* con = sockets_[i];
        if (con->isConnected()) con->dropExpiredCalls();
    }
}

} // namespace client
} // namespace pvss
} // namespace scag2
