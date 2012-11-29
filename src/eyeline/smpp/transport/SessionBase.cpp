#include "SessionBase.h"
#include "eyeline/smpp/SmppException.h"
#include "eyeline/smpp/pdu/PduInputBuf.h"
#include "eyeline/smpp/pdu/GenericNackPdu.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace smpp {

SessionBase::SessionBase( const char* sessionId, PduListener* listener ) :
Session(sessionId), listener_(listener), lastOutIdx_(0) {}


unsigned SessionBase::send( unsigned        socketId,
                            PduBuffer       pdu,
                            SendSubscriber* sub )
{
    SocketPtr sock;
    {
        MutexGuard mg(socketMon_);
        if (!socketId) {
            if (outputSockets_.empty()) {
                throw SmppException("S'%s' is not bound",getSessionId());
            }
            if (++lastOutIdx_ >= outputSockets_.size()) {
                lastOutIdx_ = 0;
            }
            sock.reset(outputSockets_[lastOutIdx_]);
        } else {
            SocketPtr* ptr = socketHash_.GetPtr(socketId);
            if (!ptr) {
                throw SmppException("Sk%u not found in S'%s'",
                                    socketId,getSessionId());
            }
            sock = *ptr;
        }
    }
    sock->send(pdu,sub);
    return sock->getSocketId();
}


void SessionBase::addSocket( Socket& socket, bool writeable )
{
    const unsigned sid = socket.getSocketId();

    MutexGuard mg(socketMon_);
    SocketPtr* ptr = socketHash_.GetPtr(sid);
    if (ptr) {
        if ( ptr->get() == &socket ) return;
        throw SmppException("logic error: Sk%u @%p is in S'%s' at addSocket(Sk%u @%p)",
                            ptr->get()->getSocketId(),
                            ptr->get(),
                            getSessionId(),
                            sid, &socket);
    }
    socketHash_.Insert( sid, SocketPtr() ).reset(&socket);
    if ( writeable ) {
        outputSockets_.push_back(&socket);
    }
    if ( socketHash_.Count() == 1 ) {
        socketMon_.notify();
    }
    socket.setSession(this);
}


bool SessionBase::removeSocket( unsigned socketId )
{
    SocketPtr ptr;
    bool res;
    {
        MutexGuard mg(socketMon_);
        if ( socketHash_.Pop(socketId,ptr) ) {
            SocketList::iterator i = std::find(outputSockets_.begin(),
                                               outputSockets_.end(),
                                               ptr.get());
            if ( i != outputSockets_.end() ) {
                outputSockets_.erase(i);
            }
            if ( !socketHash_.Count() ) {
                socketMon_.notify();
            }
        }
        res = ( socketHash_.Count() == 0 );
    }
    if (ptr.get()) {
        ptr->setSession(0);
    }
    return res;
}


void SessionBase::destroy()
{
    MutexGuard mg(socketMon_);
    SocketPtr ptr;
    int si;
    for ( SocketHash::Iterator i = socketHash_.First(); i.Next(si,ptr); ) {
        ptr->setShutdown();
        ptr->setSession(0);
    }
    socketHash_.Empty();
    outputSockets_.clear();
    socketMon_.notify();
}


/*
SocketPtr SessionBase::getSocket( unsigned socketId )
{
    MutexGuard mg(socketMon_);
    SocketPtr* ptr = socketHash_.GetPtr(socketId);
    if (ptr) {
        return *ptr;
    }
    return SocketPtr();
}
 */

void SessionBase::receivePdu( Socket& socket, const PduInputBuf& pdu )
{
    smsc_log_debug(log_,"S'%s' Sk%u received pdu seq=%u",
                   getSessionId(),socket.getSocketId(),pdu.getSeqNum());
    if (listener_) {
        listener_->receivePdu(socket,pdu);
        return;
    }
    if (!pdu.isRequest()) {
        return;
    }
    GenericNackPdu nack(Status::SYSERR,pdu.getSeqNum());
    socket.send(nack.encode(),0);
}

}
}
