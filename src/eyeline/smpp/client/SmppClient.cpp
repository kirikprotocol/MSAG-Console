#include "SmppClient.h"
#include "ClientSession.h"
#include "ClientSocket.h"
#include "eyeline/smpp/pdu/BindPdu.h"
#include "eyeline/smpp/pdu/EnquireLinkPdu.h"

namespace eyeline {
namespace smpp {

SmppClient::SmppClient( const SocketMgrConfig& cfg,
                        PduListener* listener ) :
log_(smsc::logger::Logger::getInstance("smpp.clnt")),
stopping_(true),
socketMgr_(cfg,*this,*this),
connector_(*this,*this),
listener_(listener)
{}


SmppClient::~SmppClient()
{
    stop();
}


void SmppClient::start()
{
    if (!stopping_) return;
    smsc_log_info(log_,"starting...");
    stopping_ = false;
    socketMgr_.start();
    connector_.start();
}


void SmppClient::stop()
{
    if (stopping_) return;
    smsc_log_info(log_,"stopping...");
    stopping_ = true;
    connector_.stop();
    sessionMgr_.stop();
    socketMgr_.stop();
}


void SmppClient::addSmeInfo( const ClientSmeInfo& info,
                             const char* sId )
{
    const std::string sessionId = sId ? sId : info.systemId.c_str();
    if (stopping_) {
        smsc_log_warn(log_,"not adding smeInfo=%s at stop",
                      info.systemId.c_str());
        return;
    } else if ( ! info.enabled ) {
        smsc_log_warn(log_,"not adding disabled smeInfo=%s",
                      info.systemId.c_str());
        return;
    } else if ( ! bindModeIsValid(info.bindMode) ) {
        smsc_log_warn(log_,"not adding smeInfo=%s as bindMode=%d",
                      info.systemId.c_str(),info.bindMode);
        return;
    }
    smsc_log_info(log_,"adding smeInfo=%s host=%s:%u bind=%s to S'%s'",
                  info.systemId.c_str(),info.host.c_str(),info.port,
                  bindModeToString(info.bindMode),
                  sessionId.c_str());
    SessionBasePtr ptr;
    ClientSession* cs;
    try {
        if ( ! sessionMgr_.getSession(sessionId.c_str(),ptr) ) {
            cs = new ClientSession( sessionId.c_str(),
                                    listener_);
            ptr.reset(cs);
            sessionMgr_.addSession(*ptr);
        } else {
            cs = static_cast< ClientSession* >(ptr.get());
        }
        cs->addSmeInfo( connector_, info,
                        socketMgr_.getConfig().receiveMaxSize,
                        socketMgr_.getConfig().socketWQueueSize );
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"addSme exc: %s",e.what());
        throw;
    }
}


bool SmppClient::getSession( const char* sessionId, SessionPtr& ptr )
{
    return sessionMgr_.getSession(sessionId,reinterpret_cast<SessionBasePtr&>(ptr));
}


void SmppClient::timerElapsed( msectime_type now,
                               Socket&       socket,
                               TimerType     type )
{
    smsc_log_info(log_,"timer elapsed: Sk%u type=%s",
                  socket.getSocketId(), timerTypeToString(type) );
    if (stopping_) {
        terminateSocket(socket);
        return;
    }
    switch ( type ) {
    case TIMER_CONNECT: {
        SessionBasePtr ptr(socket.getSession());
        if (!ptr) {
            smsc_log_warn(log_,"Sk%u has no session",socket.getSocketId());
        } else {
            ClientSession* cs = static_cast<ClientSession*>(ptr.get());
            ClientSocket& sock = static_cast<ClientSocket&>(socket);
            cs->connect(connector_,sock);
        }
        break;
    }
    case TIMER_ACTIVITY: {
        try {
            std::auto_ptr<Pdu> enq(new EnquireLinkPdu());
            socket.send(enq->encode(),0);
            socketMgr_.addTimer(socket,TIMER_ACTIVITY);
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"Sk%u cannot send enquirelink",
                         socket.getSocketId());
            terminateSocket(socket);
        }
        break;
    }
    case TIMER_BIND:
        if ( ! socket.getBindMode() ) {
            smsc_log_warn(log_,"Sk%u bind timeout",socket.getSocketId());
            terminateSocket(socket);
        } else {
            smsc_log_debug(log_,"Sk%u bind timer elapsed, bm=%s",
                           socket.getSocketId(),
                           bindModeToString(socket.getBindMode()));
        }
        break;
    default: {
        smsc_log_warn(log_,"Sk%u unexpected timer %u",
                      socket.getSocketId(),unsigned(type));
        terminateSocket(socket);
    }
    }
}


void SmppClient::terminateSocket( Socket& socket )
{
    smsc_log_debug(log_,"terminating Sk%u stop=%u",socket.getSocketId(),stopping_);
    try {
        socket.setShutdown();
        SessionBasePtr ses(socket.getSession());
        if (!ses) {
            smsc_log_debug(log_,"Sk%u has no session",socket.getSocketId());
        } else if ( stopping_ ) {
            ses->destroy();
        } else {
            // ClientSession* cs = static_cast< ClientSession* >(ses.get());
            socketMgr_.addTimer( socket,
                                 TIMER_CONNECT );
        }
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"terminate Sk%u exc: %s",socket.getSocketId(),e.what());
    }
}


void SmppClient::socketIsReadyToBind( Socket& socket )
{
    smsc_log_debug(log_,"Sk%u is ready to bind",socket.getSocketId());
    if ( !socket.isConnected() ) {
        smsc_log_warn(log_,"Sk%u is not connected",socket.getSocketId());
        return;
    }
    ClientSocket& cs = static_cast<ClientSocket&>(socket);
    const ClientSmeInfo& info = cs.getSmeInfo();
    if ( !bindModeIsValid(info.bindMode) ||
         info.bindMode == BINDMODE_TRANSRECV ) {
        smsc_log_warn(log_,"Sk%u has invalid bindmode=%d/%s",
                      socket.getSocketId(),
                      info.bindMode,
                      bindModeToString(info.bindMode));
        return;
    }
    socketMgr_.addSocket(socket);
    socketMgr_.addTimer(socket,TIMER_BIND);
    socketMgr_.addTimer(socket,TIMER_ACTIVITY);
    BindPdu* bind = new BindPdu(info.bindMode);
    std::auto_ptr<Pdu> bindPtr(bind);
    bind->setSystemId(info.systemId.c_str());
    bind->setPassword(info.password.c_str());
    bind->assignSeqNum();
    // bind->setSystemType();
    // bind->setAddressRange();
    // bind->setInterfaceVersion();
    // bind->setAddrTon();
    // bind->setAddrNpi();
    char buf[200];
    smsc_log_debug(log_,"Sk%u sending bind %s",
                   socket.getSocketId(),bind->toString(buf,sizeof(buf)));
    socket.send(bind->encode(),0);
}

}
}
