#include "SocketWriter.h"
#include "Core.h"
#include "Config.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

void SocketWriter::writePending()
{
    if (writePending_) return;
    smsc::core::synchronization::MutexGuard mg(mon_);
    if (writePending_) return;
    writePending_ = true;
    mon_.notify();
    while ( sockets_.Count() > 0 && writePending_ ) {
        mon_.wait(100);
    }
    return;
}


bool SocketWriter::setupSocket( Socket& socket, util::msectime_type currentTime )
{
    if ( ! socket.wantToSend(currentTime) ) return false;
    mul_.add( socket.socket() );
    return true;
}


void SocketWriter::setupFailed( util::msectime_type currentTime )
{
    if ( writePending_ ) {
        writePending_ = false;
        mon_.notify();
    } else {
        IOSocketTask::setupFailed(currentTime);
    }
}


bool SocketWriter::hasEvents()
{
    return mul_.canWrite( ready_, error_, core_.getConfig().getIOTimeout() );
}


void SocketWriter::process( Socket& socket )
{
    socket.sendData();
}


void SocketWriter::attachSocket( Socket& socket )
{
    IOSocketTask::attachSocket(socket);
    socket.setWriter(this);
}


void SocketWriter::detachSocket( Socket& socket )
{
    socket.setWriter(0);
    IOSocketTask::detachSocket(socket);
}

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2
