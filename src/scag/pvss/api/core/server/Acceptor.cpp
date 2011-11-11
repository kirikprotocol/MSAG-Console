#include "Acceptor.h"
#include "ServerCore.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

Acceptor::Acceptor(ServerConfig& config, ServerCore& core) :
SockTask(config,core,taskName()),
serverCore_(&core)
{
}


Acceptor::~Acceptor()
{
    shutdown();
}


void Acceptor::shutdown()
{
    SockTask::shutdown();
    waitUntilReleased();
    finSocks_.clear();
}


void Acceptor::init() /* throw (PvssException) */ 
{
    if ( socket_.isConnected() ) return;
    const int connectTimeout = 300;
    if ( -1 == socket_.InitServer( getConfig().getHost().c_str(),
                                   getConfig().getPort(),
                                   connectTimeout, 1, true) ) {
        // int(getConfig().getConnectTimeout())) ) {
        throw PvssException(PvssException::NOT_CONNECTED,"cannot init socket at %s:%u",
                            getConfig().getHost().c_str(),
                            unsigned(getConfig().getPort()));
    }
    if ( -1 == socket_.StartServer() ) {
        throw PvssException(PvssException::NOT_CONNECTED,"cannot init acceptor");
    }
    smsc_log_info(log_,"will accept connections on %s:%d", getConfig().getHost().c_str(), int(getConfig().getPort())&0xffff );
}


int Acceptor::setupSockets( util::msectime_type currentTime )
{
    if ( ! socket_.isConnected() ) {
        stop();
        return 1000;
    }
    if ( !finSocks_.empty() ) {
        return 0; // ready
    }
    // int tmo = int((wakeupTime_ - currentTime)/1000); // NOTE: in seconds
    // if ( tmo <= 0 ) tmo = 1;
    // check incoming sockets
    smsc::core::network::Socket* socket = socket_.Accept(1);
    if ( socket ) {
        if ( log_->isInfoEnabled() ) {
            char buf[50];
            socket->GetPeer(buf);
            smsc_log_info(log_,"incoming connection %p from: %s", socket, buf);
        }
        finSocks_.push_back(FinSock());
        FinSock& fs = finSocks_.back();
        fs.socket = socket;
        fs.connTime = util::currentTimeMillis();
    }
    return finSocks_.empty() ? 10 : 0;
}


void Acceptor::processEvents()
{
    if ( isStopping ) return;
    smsc::core::network::Multiplexer mul;
    const util::msectime_type now = util::currentTimeMillis();
    bool hasSockets = false;
    for ( size_t i = 0; i < finSocks_.size(); ) {
        FinSock& fs = finSocks_[i];
        const unsigned delta = unsigned(now - fs.connTime);
        if ( delta > getConfig().getConnectTimeout() ) {
            smsc_log_warn(log_,"socket %p connect timeout, will close",fs.socket);
            finSocks_.erase( finSocks_.begin() + i );
            continue;
        }
        ++i;
        hasSockets = true;
        smsc_log_debug(log_,"waiting to write into %p after %u msec",
                       fs.socket, unsigned(now - fs.connTime));
        mul.add( fs.socket );
    }
    if ( !hasSockets ) {
        return;
    }
    smsc::core::network::Multiplexer::SockArray ready;
    smsc::core::network::Multiplexer::SockArray error;
    mul.canWrite(ready,error,200);
    for ( int i = 0; i < error.Count(); ++i ) {
        smsc::core::network::Socket* socket = error[i];
        FinSockArray::iterator todel = std::find( finSocks_.begin(),
                                                  finSocks_.end(),
                                                  socket );
        if ( todel == finSocks_.end() ) {
            smsc_log_warn(log_,"socket %p is not found to close",socket);
            continue;
        }
        finSocks_.erase(todel);
    }
    for ( int i = 0; i < ready.Count(); ++i ) {
        smsc::core::network::Socket* socket = ready[i];
        FinSockArray::iterator todel = std::find( finSocks_.begin(),
                                                  finSocks_.end(),
                                                  socket );
        if ( todel == finSocks_.end() ) {
            smsc_log_warn(log_,"socket %p is not found to write",socket);
            continue;
        }
        PvssSocketPtr channel(new PvssSocket(*serverCore_,socket));
        todel->socket = 0; // to prevent delete
        finSocks_.erase(todel);
        // FIXME: should we check accept time of the channel ?
        bool accepted = false;
        try {
            accepted = serverCore_->acceptChannel(channel);
        } catch (...) {}
        char buf[3];
        buf[2] = 0;
        short* pbuf = reinterpret_cast<short*>(buf);
        *pbuf = htons(accepted ? Packet::CONNECT_RESPONSE_OK : Packet::CONNECT_RESPONSE_SERVER_BUSY);
        smsc_log_debug(log_,"channel %p accepted=%d, sending %s", socket, accepted ? 1 : 0, buf );
        socket->Write( buf, 2 );
        if (! accepted ) {
            socket->Close();
            // delete channel;
        }
    }
}


/*
void Acceptor::removeFinishingSocket(smsc::core::network::Socket* socket)
{
    for (int i = 0; i < finishingSockets_.Count(); ++i ) {
        if ( finishingSockets_[i].socket == socket ) {
            finishingSockets_.Delete(i);
            break;
        }
    }
}
 */

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
