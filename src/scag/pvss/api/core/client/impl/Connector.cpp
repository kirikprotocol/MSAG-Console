#include "ClientCore.h"
#include "Connector.h"
#include "scag/pvss/api/core/PvssSocket.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace client {

Connector::Connector( ClientConfig& theconfig, ClientCore& thecore ) :
SockTask(theconfig,thecore,"pvss.cnctr")
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
            const unsigned delta = unsigned(currentTime - channel.getConnectTime());
            if ( delta > core_->getConfig().getConnectTimeout() ) {
                // close on timeout
                smsc_log_warn(log_,"server has not answered on %p, will reconnect",&channel);
                channel.disconnect();
                channel.setConnectTime( currentTime );
                continue;
            }
            smsc_log_debug(log_,"channel %p is connected, waiting for server answer ...",&channel);
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
    if ( !rv ) {
        // smsc_log_debug(log_,"no channel is connecting, wakeuptime=%d",int(wakeupTime_-currentTime));
    }
    return rv;
}


void Connector::processEvents() 
{
    if ( isStopping ) return;

    // connect those sockets that are waiting for connect
    smsc_log_debug(log_,"total pending sockets to connect: %u", unsigned(pendingSockets_.Count()));
    while ( pendingSockets_.Count() > 0 ) {
        PvssSocket* channel = pendingSockets_[0];
        smsc_log_debug( log_, "connecting socket %p ...", channel->socket() );
        pendingSockets_.Delete(0);
        try {
            channel->connect();
            // move sockets to finishing
            smsc_log_debug( log_, "socket %p connected, waiting for server answer", channel->socket() );
            finishingSockets_.Push(channel);
            continue;
        } catch ( std::exception& e ) {
            // cannot connect
            smsc_log_warn(log_,"cannot connect: %s", e.what());
            core_->handleError(PvssException(PvssException::CONNECT_FAILED,"cannot connect: %s",e.what()),*channel);
        }
    }

    // process sockets that finish connecting
    smsc_log_debug(log_,"total sockets to wait for answer: %d", unsigned(finishingSockets_.Count()));
    if (finishingSockets_.Count() == 0) return;
    smsc::core::network::Multiplexer mul;
    for ( int i = 0; i < finishingSockets_.Count(); ++i ) {
        PvssSocket& channel = *finishingSockets_[i];
        mul.add( channel.socket() );
    }
    smsc::core::network::Multiplexer::SockArray ready;
    smsc::core::network::Multiplexer::SockArray error;
    mul.canRead(ready,error,200);
    smsc_log_debug(log_,"sockets: error=%u ready=%u", unsigned(error.Count()), unsigned(ready.Count()) );
    for ( int i = 0; i < error.Count(); ++i ) {
        PvssSocket& channel = *PvssSocket::fromSocket(error[i]);
        smsc_log_warn(log_,"cannot read server status on channel %p", error[i]);
        core_->handleError(PvssException(PvssException::IO_ERROR,"cannot read server status"),channel);
    }
    short serverStatus;
    for ( int i = 0; i < ready.Count(); ++i ) {
        PvssSocket& channel = *PvssSocket::fromSocket(ready[i]);
        int res = channel.socket()->Read(reinterpret_cast<char*>(&serverStatus),2);
        // may it be so that less than 2 bytes is read in an attempt ?
        if ( res != 2 ) {
            smsc_log_warn(log_,"cannot read status data on channel %p", ready[i]);
            core_->handleError(PvssException(PvssException::IO_ERROR,"cannot read server status data, res=%d",res),channel);
            continue;
        }
        serverStatus = ntohs(serverStatus);
        if ( serverStatus == Packet::CONNECT_RESPONSE_SERVER_BUSY ) {
            core_->handleError(PvssException(PvssException::SERVER_BUSY,"server busy on connect"),channel);
        } else if ( serverStatus == Packet::CONNECT_RESPONSE_OK ) {
            smsc_log_debug(log_,"server ok has been read on channel %p", ready[i]);
            core_->registerChannel(channel,util::currentTimeMillis());
            unregisterChannel(channel);
        } else {
            core_->handleError(PvssException(PvssException::CONNECT_FAILED,"unknown server status: %x",serverStatus),channel);
        }
    }
}

} // namespace client
} // namespace core
} // namespace pvss
} // namespace scag2
