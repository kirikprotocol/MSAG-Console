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
    PvssSocketPtr ptr(&socket);
    registerChannel(ptr);
}


int Connector::setupSockets(util::msectime_type currentTime)
{
    pendingSockets_.clear();
    finishingSockets_.clear();
    int rv = 300;
    for ( size_t i = 0; i < workingSockets_.size(); ++i ) {
        PvssSocketPtr& channel = workingSockets_[i];
        if ( channel->isConnected() ) {
            const unsigned delta = unsigned(currentTime - channel->getConnectTime());
            if ( delta > core_->getConfig().getConnectTimeout() ) {
                // close on timeout
                smsc_log_warn(log_,"server has not answered on %p sock=%p, will reconnect",
                              channel.get(), channel->getSocket());
                channel->getSocket()->Close();
                channel->setConnectTime( currentTime );
                continue;
            }
            smsc_log_debug(log_,"channel %p sock=%p is connected, waiting for server answer ...",
                           channel.get(), channel->getSocket());
            finishingSockets_.push_back( channel );
            rv = 0;
        } else {
            const int ct = int(channel->getConnectTime() - currentTime);
            if ( ct <= 0 ) {
                pendingSockets_.push_back( channel );
                rv = 0;
            } else if ( ct < rv ) {
                rv = ct;
            }
        }
    }
    return rv;
}


void Connector::processEvents() 
{
    if ( isStopping ) return;

    // connect those sockets that are waiting for connect
    smsc_log_debug(log_,"total pending sockets to connect: %u", unsigned(pendingSockets_.size()));
    while ( pendingSockets_.size() > 0 ) {
        PvssSocketPtr channel = pendingSockets_.front();
        smsc_log_debug( log_, "connecting socket %p sock=%p...", channel.get(), channel->getSocket() );
        pendingSockets_.erase( pendingSockets_.begin() );
        try {
            channel->connect();
            // move sockets to finishing
            smsc_log_debug( log_, "channel %p sock=%p connected, waiting for server answer",
                            channel.get(), channel->getSocket() );
            finishingSockets_.push_back(channel);
            continue;
        } catch ( std::exception& e ) {
            // cannot connect
            smsc_log_warn(log_,"cannot connect: %s", e.what());
            core_->handleError(PvssException(PvssException::CONNECT_FAILED,"cannot connect: %s",
                                             e.what()),*channel);
        }
    }

    // process sockets that finish connecting
    smsc_log_debug(log_,"total sockets to wait for answer: %d", unsigned(finishingSockets_.size()));
    if (finishingSockets_.empty()) return;
    smsc::core::network::Multiplexer mul;
    for ( size_t i = 0; i < finishingSockets_.size(); ++i ) {
        PvssSocketPtr& channel = finishingSockets_[i];
        mul.add( channel->getSocket() );
    }
    smsc::core::network::Multiplexer::SockArray ready;
    smsc::core::network::Multiplexer::SockArray error;
    mul.canRead(ready,error,200);
    smsc_log_debug(log_,"sockets: error=%u ready=%u", unsigned(error.Count()), unsigned(ready.Count()) );
    for ( int i = 0; i < error.Count(); ++i ) {
        PvssSocket& channel = *PvssSocket::fromSocket(error[i]);
        smsc_log_warn(log_,"cannot read server status on channel %p sock=%p", &channel, channel.getSocket());
        core_->handleError(PvssException(PvssException::IO_ERROR,"cannot read server status"),channel);
    }
    short serverStatus;
    for ( int i = 0; i < ready.Count(); ++i ) {
        PvssSocketPtr channel( PvssSocket::fromSocket(ready[i]) );
        int res = channel->getSocket()->Read(reinterpret_cast<char*>(&serverStatus),2);
        // may it be so that less than 2 bytes is read in an attempt ?
        if ( res != 2 ) {
            smsc_log_warn(log_,"cannot read status data on channel %p sock=%p", channel.get(), channel->getSocket());
            core_->handleError(PvssException(PvssException::IO_ERROR,"cannot read server status data, res=%d",res),*channel);
            continue;
        }
        serverStatus = ntohs(serverStatus);
        if ( serverStatus == Packet::CONNECT_RESPONSE_SERVER_BUSY ) {
            core_->handleError(PvssException(PvssException::SERVER_BUSY,"server busy on connect"),*channel);
        } else if ( serverStatus == Packet::CONNECT_RESPONSE_OK ) {
            smsc_log_debug(log_,"server ok has been read on channel %p sock=%p", 
                           channel.get(), channel->getSocket());
            core_->registerChannel(channel,util::currentTimeMillis());
            unregisterChannel(*channel);
        } else {
            core_->handleError(PvssException(PvssException::CONNECT_FAILED,"unknown server status: %x",serverStatus),*channel);
        }
    }
}

} // namespace client
} // namespace core
} // namespace pvss
} // namespace scag2
