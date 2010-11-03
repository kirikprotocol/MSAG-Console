#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/tcpsrv/ICSTcpSrv.hpp"

namespace smsc  {
namespace inman {
namespace tcpsrv {
/* ************************************************************************** *
 * class ICSTcpServer implementation:
 * ************************************************************************** */

// --------------------------------------------------------------------------
// -- TCPServerITF interface mthods
// --------------------------------------------------------------------------
bool ICSTcpServer::registerProtocol(const INPCommandSetAC * cmd_set, ConnServiceITF * conn_srv)
{
    MutexGuard  grd(_sync);
    INPSerializer::getInstance()->registerCmdSet(cmd_set);
    std::pair<ProtocolsMap::iterator, bool> res =
        csSrv.insert(ProtocolsMap::value_type(cmd_set->CsId(), conn_srv));
    return res.second;
}


// --------------------------------------------------------------------------
// -- ServerListenerITF interface mthods
// --------------------------------------------------------------------------
ConnectAC * ICSTcpServer::onConnectOpening(Server* srv, Socket* sock)
{
    MutexGuard  grd(_sync);
    Connect * conn = new Connect(sock, INPSerializer::getInstance());
    conn->addListener(this);
    return conn;
}

//Remote point ends connection
void ICSTcpServer::onConnectClosing(Server* srv, ConnectAC* conn)
{
    closeSession(conn->getId());
}

//throws CustomException
void ICSTcpServer::onServerShutdown(Server* srv, Server::ShutdownReason reason)
{
    tcpSrv->removeListener(this);
    MutexGuard  grd(_sync);
    _icsState = ICServiceAC::icsStInited;
    smsc_log_debug(logger, "%s: TCP server shutdowning, reason %d", _logId, reason);
}

// --------------------------------------------------------------------------
// -- ConnectListenerITF interface mthods
// --------------------------------------------------------------------------
//Creates/Restores session (creates/rebinds ConnectManager)
void ICSTcpServer::onPacketReceived(Connect* conn, 
                                    std::auto_ptr<SerializablePacketAC>& recv_cmd)
        /*throw(std::exception)*/
{
    INPPacketAC* pck = static_cast<INPPacketAC*>(recv_cmd.get());
    SessionInfo  newSess(conn);
    ConnectManagerAC * hdl = NULL;
    {
        MutexGuard tmp(_sync);
        newSess.pCs = INPSerializer::getInstance()->commandSet(pck->pCmd()->Id());
        if (!newSess.pCs)
            throw CustomException("%s: unsupported INPPacket: %u", _logId,
                                  pck->pCmd()->Id());

        ProtocolsMap::iterator it = csSrv.find(newSess.pCs->CsId());
        newSess.connSrv = (it != csSrv.end()) ? it->second : NULL;
        if (!newSess.connSrv)
            throw CustomException("%s: unregistered protocol: %s", _logId,
                                  newSess.pCs->CsName());

        //NOTE: session restoration is not supported for now, so
        //      just create new session.
        //NOTE: only commands with HDR_DIALOG is supported for now
        //      (bindSockId session type)
        newSess.sId = ++lastSessId;
        if (!(hdl = newSess.connSrv->getConnManager(newSess.sId, conn)))
            throw CustomException("%s: failed to get ConnManager for: %s", _logId,
                                  newSess.pCs->CsName());
        sockets.insert(SocketsMap::value_type(conn->getId(), newSess.sId));
        sessions.insert(SessionsMap::value_type(newSess.sId, newSess));
        smsc_log_info(logger, "%s: New %s session[%u] on Connect[%u] created", _logId,
                      newSess.pCs->CsName(), newSess.sId, conn->getId());
    }
    conn->folowUp(this, hdl);
    conn->removeListener(this);
}

//NOTE: session restoration is not supported for now, so just delete session
void ICSTcpServer::onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc)
{
    conn->removeListener(this);
    closeSession(conn->getId());
}

// --------------------------------------------------------------------------
// -- Private mthods
// --------------------------------------------------------------------------
//NOTE: must be called with _sync being unlocked
void ICSTcpServer::closeSession(unsigned sockId)
{
     {
        MutexGuard grd(_sync);
        SocketsMap::iterator it = sockets.find(sockId);
        if (it == sockets.end() || !it->second)
            return;
        unsigned sesId = it->second;
        sockets.erase(it);

        SessionsMap::iterator sit = sessions.find(sesId);
        if (sit != sessions.end()) {
            SessionInfo & sess = sit->second;
            smsc_log_info(logger, "%s: closing %s session[%u] on Connect[%u]", _logId,
                            sess.pCs->CsName(), sess.sId, sockId);
            sess.rlseConnManager();
            sessions.erase(sit);
        }
    }
    return;
}

} //tcpsrv
} //inman
} //smsc

