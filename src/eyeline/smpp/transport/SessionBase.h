#ifndef _EYELINE_SMPP_SESSIONBASE_H
#define _EYELINE_SMPP_SESSIONBASE_H

#include <vector>
#include <memory>
#include "eyeline/smpp/Session.h"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "Socket.h"

namespace eyeline {
namespace smpp {

/// inner class owning its sockets
class SessionBase : public Session
{
public:
    SessionBase( const char* sessionId, PduListener* listener = 0 );

    /// send pdu into network or throw an exception.
    /// see comments at base class.
    virtual unsigned send( unsigned        socketId,
                           PduBuffer       buffer,
                           SendSubscriber* sub = 0 );

    /// return true when there is no more sockets
    virtual bool removeSocket( unsigned socketId );

    /// send destroy signal, i.e. remove all sockets
    void destroy();

    /// receive pdu from network
    virtual void receivePdu( Socket& socket, const PduInputBuf& pdu );

    /// check if session has output sockets
    virtual bool isBound()
    {
        smsc::core::synchronization::MutexGuard mg(socketMon_);
        for ( std::vector< Socket* >::const_iterator i = outputSockets_.begin(),
              ie = outputSockets_.end(); i != ie; ++i ) {
            if ( (*i)->getBindMode() != BindMode(0) ) return true;
        }
        return false;
    }

protected:
    void addSocket( Socket& socket, bool writeable );

protected:
    typedef smsc::core::buffers::IntHash< SocketPtr > SocketHash;
    typedef std::vector< Socket* > SocketList;

    PduListener*                              listener_;
    smsc::core::synchronization::EventMonitor socketMon_;
    SocketHash                                socketHash_;    // all sockets
    SocketList                                outputSockets_; // output only sockets
    unsigned                                  lastOutIdx_;
};

typedef eyeline::informer::EmbedRefPtr< SessionBase > SessionBasePtr;

}
}


#endif
