#include "ClientSession.h"
#include "ClientSocket.h"
#include "SmppConnector.h"
#include "eyeline/smpp/pdu/BindPdu.h"
#include "eyeline/smpp/pdu/PduInputBuf.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace smpp {


ClientSession::ClientSession( const char*  sessionId,
                              PduListener* listener ) :
SessionBase(sessionId,listener)
{}


void ClientSession::connect( SmppConnector& conn, ClientSocket& socket )
{
    smsc_log_debug(log_,"S'%s' connect Sk%u",getSessionId(),socket.getSocketId());
    conn.addSocket(socket);
}


void ClientSession::addSmeInfo( SmppConnector& conn,
                                const ClientSmeInfo& info,
                                size_t maxrsize, size_t maxwqueue )
{
    for ( unsigned i = 0; i < info.nsockets; ++i ) {
        // create all sockets
        // the client can write to TRX or TX socket
        const unsigned wbits = (BINDMODE_TRANSCEIVER|BINDMODE_TRANSMITTER);
        ClientSocket* cs;
        if ( info.bindMode != BINDMODE_TRANSRECV ) {
            SocketPtr ptr( cs = new ClientSocket(info,maxrsize,maxwqueue) );
            addSocket(*ptr, info.bindMode & wbits);
            connect(conn,*cs);
        } else {
            ClientSmeInfo csi(info);
            csi.bindMode = BINDMODE_TRANSMITTER;
            SocketPtr ptr( cs = new ClientSocket(csi,maxrsize,maxwqueue) );
            addSocket( *ptr, csi.bindMode & wbits );
            connect(conn,*cs);
            csi.bindMode = BINDMODE_RECEIVER;
            ptr.reset( cs = new ClientSocket(csi,maxrsize,maxwqueue) );
            addSocket( *ptr, csi.bindMode & wbits );
            connect(conn,*cs);
        }
    }
}


void ClientSession::receivePdu( Socket& socket, const PduInputBuf& pdu )
{
    switch (pdu.getPduTypeId()) {
    case BIND_RECEIVER_RESP:
    case BIND_TRANSMITTER_RESP:
    case BIND_TRANSCEIVER_RESP: {
        if ( pdu.getStatus() != Status::OK ) {
            smsc_log_warn(log_,"Sk%u received bind_resp with status %u",
                          socket.getSocketId(), pdu.getStatus());
            socket.setShutdown();
            return;
        }

        std::auto_ptr<BindRespPdu> resp(static_cast<BindRespPdu*>(pdu.decode()));
        char buf[200];
        smsc_log_debug(log_,"Sk%u bind_resp received %s",
                       socket.getSocketId(),resp->toString(buf,sizeof(buf)));
        ClientSocket& sock = static_cast<ClientSocket&>(socket);
        const BindMode bm = sock.getSmeInfo().bindMode;
        sock.setBindMode(bm);
        smsc_log_info(log_,"Sk%u bound as '%s' to '%s' bm=%s",
                      sock.getSocketId(),
                      sock.getSmeInfo().systemId.c_str(),
                      getSessionId(),
                      bindModeToString(bm));
        break;
    }
    default:
        SessionBase::receivePdu(socket,pdu);
    }
}

}
}
