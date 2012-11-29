#ifndef _EYELINE_SMPP_SMPPSERVER_H
#define _EYELINE_SMPP_SMPPSERVER_H

#include "eyeline/smpp/ServerIface.h"
#include "eyeline/smpp/transport/SocketCloser.h"
#include "eyeline/smpp/transport/SessionBase.h"
#include "eyeline/smpp/transport/SessionMgr.h"
#include "eyeline/smpp/transport/SocketMgr.h"
#include "eyeline/smpp/transport/SocketBinder.h"
#include "SmppAcceptor.h"
#include "eyeline/smpp/SmeInfo.h"

namespace eyeline {
namespace smpp {


/// smpp server
class SmppServer : public ServerIface, public TimerSubscriber, public PduListener, public SocketCloser, public SocketBinder
{
    struct SessionSmeInfo : public SmeInfo 
    {
        std::string sessionId;
    };

    class UnboundSession;

public:
    SmppServer( const char* host,
                int         port,
                const SocketMgrConfig& cfg,
                PduListener* external = 0 );
    virtual ~SmppServer();

    /// the second parameter may be used
    /// to create a sessions combining several smeinfos.
    void addSmeInfo( const SmeInfo& smeInfo, const char* sessionId = 0 );
    void updateSmeInfo( const SmeInfo& smeInfo );

    void start();
    void stop();

    bool getSession( const char* sessionId, SessionPtr& ptr );

protected:
    virtual void socketIsReadyToBind( Socket& socket );
    virtual void receivePdu( Socket& socket, const PduInputBuf& pdu );
    virtual void timerElapsed( msectime_type now,
                               Socket&       socket,
                               TimerType     type );
    virtual void terminateSocket( Socket& socket );

private:
    smsc::logger::Logger* log_;
    bool                  stopping_;
    SocketMgr             socketMgr_;
    SessionBasePtr        unbound_;
    SessionMgr            sessionMgr_;
    SmppAcceptor          acceptor_;
    PduListener*          listener_; // not owned

    smsc::core::synchronization::Mutex   smeLock_;
    smsc::core::buffers::Hash< SessionSmeInfo > smeInfos_;
};

}
}

#endif
