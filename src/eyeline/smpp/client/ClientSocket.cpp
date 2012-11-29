#include "ClientSocket.h"
#include "eyeline/smpp/SmppException.h"

namespace eyeline {
namespace smpp {

void ClientSocket::connect()
{
    smsc_log_debug(log_,"Sk%u connecting to %s:%u",
                   getSocketId(),info_.host.c_str(),info_.port);
    socket_->Close();
    if (!inited_) {
        if ( socket_->Init(info_.host.c_str(),info_.port,0) ) {
            char ebuf[100];
            throw SmppException("Sk%u connect(%s:%u) exc at Init: err=%u %s",
                                getSocketId(),
                                info_.host.c_str(),info_.port,
                                errno,STRERROR(errno,ebuf,sizeof(ebuf)));
        }
        inited_ = true;
    }
    socket_->setConnectTimeout(0);
    if ( socket_->Connect(true) ) {
        char ebuf[100];
        throw SmppException("Sk%u connect(%s,%u) exc: err=%u %s",
                            getSocketId(),
                            info_.host.c_str(),info_.port,
                            errno,STRERROR(errno,ebuf,sizeof(ebuf)));
    }
    unsetShutdown();
    smsc_log_debug(log_,"Sk%u connected=%u",getSocketId(),socket_->isConnected());
}

}
}
