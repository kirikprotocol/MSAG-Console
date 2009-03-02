#include "ClientCore.h"
#include "Connector.h"
#include "scag/pvss/api/core/PvssSocket.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace client {

Connector::Connector( Config& theconfig, Core& thecore ) :
SockTask(theconfig,thecore,"pvss.ctask"),
wakeupTime_(0)
{
}

void Connector::shutdown()
{
    SockTask::shutdown();
    waitUntilReleased();
}


void Connector::connectChannel(PvssSocket& socket, util::msectime_type startConnectTime)
{
    if (isStopping) return;
    if (socket.isConnected()) return;
    socket.setConnectTime(startConnectTime);
    registerChannel(socket);
}


bool Connector::setupSockets(util::msectime_type currentTime)
{
    pendingSockets_.Empty();
    finishingSockets_.Empty();
    bool rv = false;
    for ( int i = 0; i < sockets_.Count(); ++i ) {
        PvssSocket& channel = *sockets_[i];
        if ( channel.isConnected() ) {
            finishingSockets_.Push( &channel );
            wakeupTime_ = currentTime;
            rv = true;
        } else {
            const util::msectime_type ct = channel.getConnectTime();
            if ( ct <= currentTime ) {
                pendingSockets_.Push( &channel );
                rv = true;
            } else if ( ct < wakeupTime_ ) {
                wakeupTime_ = ct;
            }
        }
    }
    return rv;
}


void Connector::processEvents() 
{
    if ( isStopping ) return;

    // connect those sockets that are waiting for connect
    while ( pendingSockets_.Count() > 0 ) {
        PvssSocket& channel = *pendingSockets_[0];
        smsc_log_debug( log_, "Connecting channel %p ...", &channel );
        pendingSockets_.Delete(0);
        try {
            channel.connect();
            // move sockets to finishing
            finishingSockets_.Push(&channel);
            continue;
        } catch ( exceptions::IOException& e ) {
            // cannot connect
            core->handleError(PvssException(PvssException::CONNECT_FAILED,"cannot connect: %s",e.what()),channel);
        }
    }

    // process sockets that finish connecting
    if (finishingSockets_.Count() == 0) return;
    smsc::core::network::Multiplexer mul;
    for ( int i = 0; i < finishingSockets_.Count(); ++i ) {
        PvssSocket& channel = *finishingSockets_[i];
        mul.add( channel.socket() );
    }
    smsc::core::network::Multiplexer::SockArray ready;
    smsc::core::network::Multiplexer::SockArray error;
    mul.canRead(ready,error,200);
    for ( int i = 0; i < error.Count(); ++i ) {
        PvssSocket& channel = *PvssSocket::fromSocket(error[i]);
        core->handleError(PvssException(PvssException::IO_ERROR,"cannot read server status"),channel);
    }
    std::string serverStatus;
    serverStatus.resize(2);
    for ( int i = 0; i < ready.Count(); ++i ) {
        PvssSocket& channel = *PvssSocket::fromSocket(ready[i]);
        int res = channel.socket()->Read(const_cast<char*>(serverStatus.c_str()),2);
        // FIXME: may it be so that less than 2 bytes is read in an attempt ?
        if ( res != 2 ) {
            core->handleError(PvssException(PvssException::IO_ERROR,"cannot read server status data, res=%d",res),channel);
            continue;
        }
        if ( serverStatus == "SB" ) {
            core->handleError(PvssException(PvssException::SERVER_BUSY,"server busy on connect"),channel);
        } else if ( serverStatus == "OK" ) {
            core->registerChannel(channel,util::currentTimeMillis());
            unregisterChannel(channel);
        } else {
            core->handleError(PvssException(PvssException::CONNECT_FAILED,"unknown server status '%s'",serverStatus.c_str()),channel);
        }
    }
}

} // namespace client
} // namespace core
} // namespace pvss
} // namespace scag2
