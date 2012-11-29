#include <assert.h>
#include "SmppServer.h"
#include "ServerSession.h"
#include "eyeline/smpp/pdu/Pdu.h"
#include "eyeline/smpp/pdu/BindPdu.h"
#include "eyeline/smpp/pdu/PduInputBuf.h"
#include "eyeline/smpp/pdu/EnquireLinkPdu.h"
#include "eyeline/smpp/SmppException.h"
#include "eyeline/smpp/transport/SmppWriter.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace smpp {

class SmppServer::UnboundSession : public SessionBase
{
public:
    UnboundSession( const char* sesId, PduListener* list ) :
    SessionBase(sesId,list) {}
    void addUnboundSocket( Socket& socket ) {
        addSocket(socket,false);
    }
};

// ======================================================


SmppServer::SmppServer( const char* host,
                        int         port,
                        const SocketMgrConfig& cfg,
                        PduListener* listener ) :
log_(smsc::logger::Logger::getInstance("smpp.srv")),
stopping_(true),
socketMgr_(cfg,*this,*this),
unbound_(new UnboundSession("<unbound>",this)),
acceptor_(*this,host,port,cfg.receiveMaxSize,cfg.socketWQueueSize),
listener_(listener)
{}


SmppServer::~SmppServer()
{
    stop();
}


void SmppServer::addSmeInfo( const SmeInfo& smeInfo, const char* sessionId )
{
    {
        MutexGuard mg(smeLock_);
        if ( smeInfos_.Exists(smeInfo.systemId.c_str()) ) {
            throw SmppException("addSmeInfo exc: smeInfo=%s already exists",smeInfo.systemId.c_str());
        }
        SessionSmeInfo& info = *smeInfos_.SetItem(smeInfo.systemId.c_str(),SessionSmeInfo());
        static_cast<SmeInfo&>(info) = smeInfo;
        info.sessionId = sessionId ? sessionId : smeInfo.systemId;
    }
    smsc_log_info(log_,"smeInfo=%s S'%s' enabled=%d added",
                  smeInfo.systemId.c_str(),
                  sessionId ? sessionId : smeInfo.systemId.c_str(),
                  smeInfo.enabled);
}


void SmppServer::updateSmeInfo( const SmeInfo& smeInfo )
{
    {
        MutexGuard mg(smeLock_);
        SessionSmeInfo* ptr = smeInfos_.GetPtr(smeInfo.systemId.c_str());
        if (!ptr) {
            throw SmppException("updateSmeInfo exc: smeInfo=%s not found",smeInfo.systemId.c_str());
        }
        static_cast<SmeInfo&>(*ptr) = smeInfo;
        // FIXME: update session by sme systemId
    }
    smsc_log_info(log_,"smeInfo=%s enabled=%d updated",smeInfo.systemId.c_str(),smeInfo.enabled);
}


void SmppServer::start()
{
    if (!stopping_) return;
    smsc_log_info(log_,"starting...");
    stopping_ = false;
    socketMgr_.start();
    acceptor_.start();
}


void SmppServer::stop()
{
    if (stopping_) return;
    smsc_log_info(log_,"stopping...");
    stopping_ = true;
    acceptor_.stop();
    sessionMgr_.stop();
    socketMgr_.stop();
}


bool SmppServer::getSession( const char* sessionId, SessionPtr& ptr )
{
    return sessionMgr_.getSession(sessionId,reinterpret_cast<SessionBasePtr&>(ptr));
}


void SmppServer::socketIsReadyToBind( Socket& socket )
{
    smsc_log_info(log_,"Sk%u is ready to bind",socket.getSocketId());
    if (!socket.isConnected()) {
        smsc_log_warn(log_,"Sk%u is not connected",socket.getSocketId());
        return;
    }
    // we are not going to write anything to the socket from unbound session
    static_cast<UnboundSession*>(unbound_.get())->addUnboundSocket(socket);
    socketMgr_.addSocket(socket);
    socketMgr_.addTimer(socket,TIMER_BIND);
}


void SmppServer::receivePdu( Socket& socket, const PduInputBuf& pdu )
{
    switch ( PduType(pdu.getPduTypeId()) ) {
    case BIND_RECEIVER    :
    case BIND_TRANSMITTER :
    case BIND_TRANSCEIVER : break;
    default: {
        std::auto_ptr<Pdu> resp(pdu.createResponse(Status::INVBNDSTS));
        socket.send(resp->encode(),0);
        return;
    }
    }
    std::auto_ptr<BindPdu> bind(static_cast< BindPdu* >(pdu.decode()));
    uint32_t respStatus = Status::INVSYSID;
    SessionSmeInfo info;
    do {
        // FIXME: check white list, etc.
        MutexGuard mg(smeLock_);
        SessionSmeInfo* ptr = smeInfos_.GetPtr(bind->getSystemId());
        if (!ptr) break;
        if (!ptr->enabled) break;
        if (ptr->password != bind->getPassword()) {
            respStatus = Status::INVPASWD;
            break;
        }
        if ( (bind->getBindMode() & ptr->bindMode) == 0 ) {
            // bind mode does not match
            respStatus = Status::INVBNDSTS;
            break;
        }
        respStatus = Status::OK;
        info = *ptr;
    } while (false);

    if (respStatus == uint32_t(Status::OK) ) {

        // the server can write to TRX or RX sockets
        // const bool writeable = 
        // ( bind->getBindMode() & (BINDMODE_TRANSCEIVER|BINDMODE_RECEIVER) );
        socket.setBindMode(bind->getBindMode());

        // trying to get/create session
        SessionBasePtr ses;
        const bool found = sessionMgr_.getSession(info.sessionId.c_str(),ses);
        if (!found) {
            ses.reset( new ServerSession(info.sessionId.c_str(),listener_) );
        }
        assert(ses.get());
        ServerSession* session = static_cast<ServerSession*>(ses.get());
        try {
            session->addServerSocket(socket,info);
            smsc_log_info(log_,"Sk%u bound as '%s' to S'%s'",
                          socket.getSocketId(),
                          bind->getSystemId(),
                          ses->getSessionId());
            if (!found) {
                sessionMgr_.addSession(*ses);
            }
        } catch ( std::exception& e ) {
            // could not add socket to session
            smsc_log_warn(log_,"S'%s' addSock exc: %s",
                          session->getSessionId(),e.what());
            respStatus = Status::INVBNDSTS;
        }
    }
    socketMgr_.addTimer(socket,TIMER_ACTIVITY);
    {
        std::auto_ptr<Pdu> resp(bind->createResponse(respStatus));
        socket.send(resp->encode(),0);
    }
}


void SmppServer::timerElapsed( msectime_type now,
                               Socket&       socket,
                               TimerType     type )
{
    smsc_log_info(log_,"timer elapsed: Sk%u type=%s",
                  socket.getSocketId(), timerTypeToString(type) );
    switch (type) {
    case (TIMER_BIND) :
        if ( socket.getBindMode() == BindMode(0) ) {
            smsc_log_warn(log_,"Sk%u is not bound, terminating",
                          socket.getSocketId());
            terminateSocket(socket);
        }
        break;
    case (TIMER_ACTIVITY) :
        try {
            if ( socket.isConnected() ) {
                std::auto_ptr<Pdu> enq(new EnquireLinkPdu());
                socket.send(enq->encode(),0);
                socketMgr_.addTimer(socket,TIMER_ACTIVITY);
            }
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"Sk%u cannot send enquirelink",
                          socket.getSocketId());
            terminateSocket(socket);
        }
        break;
    case (TIMER_CONNECT) :
    default: {
        smsc_log_warn(log_,"Sk%u unexpected timer %u",
                      socket.getSocketId(),unsigned(type));
        terminateSocket(socket);
    }
    }
}


void SmppServer::terminateSocket( Socket& socket )
{
    smsc_log_info(log_,"Sk%u terminating",socket.getSocketId());
    try {
        SessionBasePtr ptr = socket.getSession();
        socketMgr_.removeTimers(socket);
        socket.setShutdown();
        eyeline::informer::EmbedRefPtr< SmppWriter > writer(socket.getWriter());
        if (writer.get()) {
            writer->wakeUp();
        }
        if ( ptr.get() && 
             ptr->removeSocket(socket.getSocketId()) &&
             ptr != unbound_ ) {
            sessionMgr_.removeSession(ptr->getSessionId());
        }
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"terminate Sk%u exc: %s",socket.getSocketId(),e.what());
    }
}

}
}
