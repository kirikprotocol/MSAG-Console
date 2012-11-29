#ifndef _EYELINE_SMPP_SMPPCLIENT_H
#define _EYELINE_SMPP_SMPPCLIENT_H

#include "eyeline/smpp/ClientIface.h"
#include "eyeline/smpp/transport/SocketCloser.h"
#include "eyeline/smpp/transport/SocketMgr.h"
#include "eyeline/smpp/transport/SessionMgr.h"
#include "eyeline/smpp/transport/SocketBinder.h"
#include "ClientSmeInfo.h"
#include "SmppConnector.h"

namespace eyeline {
namespace smpp {

/// smpp server
class SmppClient : public ClientIface, public SocketCloser, public TimerSubscriber, public SocketBinder
{
public:
    SmppClient( const SocketMgrConfig& cfg,
                PduListener* external = 0 );
    virtual ~SmppClient();
    void start();
    void stop();

    void addSmeInfo( const ClientSmeInfo& info, const char* sessionId = 0 );
    
    /// get session by id
    bool getSession( const char* sessionId, SessionPtr& ptr );

protected:
    // virtual void receivePdu( Socket& socket, Pdu* pdu );
    virtual void timerElapsed( msectime_type now,
                               Socket&       socket,
                               TimerType     type );
    virtual void terminateSocket( Socket& socket );
    virtual void socketIsReadyToBind( Socket& socket );

private:
    smsc::logger::Logger* log_;
    bool                  stopping_;
    SocketMgr             socketMgr_;
    SessionMgr            sessionMgr_;
    SmppConnector         connector_;
    PduListener*          listener_;
};

}
}

#endif
