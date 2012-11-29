#include "ServerSession.h"
#include "eyeline/smpp/SmppException.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace smpp {

ServerSession::SSI::SSI( const SmeInfo& info ) :
systemId(info.systemId), nsockr(0), nsockw(0)
{
}


ServerSession::~ServerSession()
{
    ssiMap_.Empty();
    char* key;
    SSI* ptr;
    for ( smsc::core::buffers::Hash<SSI*>::Iterator i(&ssiHash_); i.Next(key,ptr); ) {
        delete ptr;
    }
    ssiHash_.Empty();
}


bool ServerSession::SSI::addSocket( const SmeInfo& info, BindMode bindMode )
{
    if ( isWritable(bindMode) ) {
        if ( nsockw >= info.nsockets ) {
            return false;
        }
        ++nsockw;
    } else {
        if ( nsockr >= info.nsockets ) {
            return false;
        }
        ++nsockr;
    }
    return true;
}


bool ServerSession::SSI::removeSocket( BindMode bindMode )
{
    if ( isWritable(bindMode) ) {
        --nsockw;
    } else {
        --nsockr;
    }
    return ( nsockw == 0 && nsockr == 0 );
}


bool ServerSession::removeSocket( unsigned socketId )
{
    MutexGuard mg(ssiLock_);
    SSILink link;
    if (!ssiMap_.Pop(socketId,link)) {
        return SessionBase::removeSocket(socketId);
    }
    if ( link.ssi->removeSocket(link.bindMode) ) {
        ssiHash_.Delete(link.ssi->systemId.c_str());
        delete link.ssi;
    }
    return SessionBase::removeSocket(socketId);
}



void ServerSession::addServerSocket( Socket& socket, 
                                     const SmeInfo& info )
{
    const BindMode bindMode = socket.getBindMode();
    if (bindMode == BindMode(0)) {
        throw SmppException("Sk%u is not bound",socket.getSocketId());
    }
    MutexGuard mg(ssiLock_);
    SSILink* ptr = ssiMap_.GetPtr(socket.getSocketId());
    if (ptr) {
        throw SmppException("logic err: Sk%u is in S'%s' already at addSocket",
                            socket.getSocketId(), getSessionId());
    }

    SSI** ssiptr = ssiHash_.GetPtr(info.systemId.c_str());
    std::auto_ptr<SSI> ssihold;
    SSI* ssi;
    if (!ssiptr) {
        ssihold.reset(new SSI(info));
        ssi = ssihold.get();
    } else {
        ssi = *ssiptr;
    }
    if ( ! ssi->addSocket(info,bindMode) ) {
        throw SmppException("S'%s' too many sockets via '%s': info.bm=%s,bm=%s,r=%u,w=%u",
                            getSessionId(),
                            info.systemId.c_str(),
                            bindModeToString(info.bindMode),
                            bindModeToString(bindMode),
                            ssi->nsockr,
                            ssi->nsockw);
    }

    // success
    if (!ssiptr) {
        ssiHash_.Insert(info.systemId.c_str(),ssihold.release());
    }
    SSILink& link = ssiMap_.Insert(socket.getSocketId(),SSILink());
    link.ssi = ssi;
    link.bindMode = bindMode;
    addSocket(socket,SSI::isWritable(bindMode));
}

}
}
