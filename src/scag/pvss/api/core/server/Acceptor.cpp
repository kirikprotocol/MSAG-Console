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
    for ( int i = 0; i < finishingSockets_.Count(); ++i ) {
        const FinSock& fs = finishingSockets_[i];
        fs.socket->Close();
        delete fs.socket;
    }
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


bool Acceptor::setupSockets( util::msectime_type currentTime )
{
    if ( ! socket_.isConnected() ) {
        stop();
        return false;
    }
    if ( finishingSockets_.Count() > 0 ) wakeupTime_ = currentTime; // don't block on accept
    int tmo = int((wakeupTime_ - currentTime)/1000); // NOTE: in seconds
    if ( tmo <= 0 ) tmo = 1;
    // check incoming sockets
    smsc::core::network::Socket* socket = socket_.Accept(tmo);
    if ( socket ) {
        if ( log_->isInfoEnabled() ) {
            char buf[50];
            socket->GetPeer(buf);
            smsc_log_info(log_,"incoming connection %p from: %s", socket, buf);
        }
        FinSock fs;
        fs.socket = socket;
        fs.connTime = util::currentTimeMillis();
        finishingSockets_.Push(fs);
    }
    return finishingSockets_.Count() > 0;
}


void Acceptor::processEvents()
{
    if ( isStopping ) return;
    smsc::core::network::Multiplexer mul;
    const util::msectime_type now = util::currentTimeMillis();
    bool hasSockets = false;
    for ( int i = 0; i < finishingSockets_.Count(); ++i ) {
        FinSock& fs = finishingSockets_[i];
        const unsigned delta = unsigned(now - fs.connTime);
        smsc::core::network::Socket* socket = fs.socket;
        if ( delta > getConfig().getConnectTimeout() ) {
            smsc_log_warn(log_,"socket %p connect timeout, will close",socket);
            socket->Close();
            removeFinishingSocket(socket);
            delete socket;
            continue;
        }
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
        removeFinishingSocket(socket);
        socket->Close();
        delete socket;
    }
    for ( int i = 0; i < ready.Count(); ++i ) {
        smsc::core::network::Socket* socket = ready[i];
        removeFinishingSocket(socket);
        PvssSocket* channel = new PvssSocket(*serverCore_,socket);
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
            delete channel;
        }
    }
}


void Acceptor::removeFinishingSocket(smsc::core::network::Socket* socket)
{
    for (int i = 0; i < finishingSockets_.Count(); ++i ) {
        if ( finishingSockets_[i].socket == socket ) {
            finishingSockets_.Delete(i);
            break;
        }
    }
}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
