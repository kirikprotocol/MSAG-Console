/* ************************************************************************* *
 * TCP Server service.
 * ************************************************************************* */
#ifndef __INMAN_ICS_TCPSERVER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TCPSERVER_HPP

#include "inman/interaction/server.hpp"
#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/tcpsrv/TCPSrvDefs.hpp"

namespace smsc {
namespace inman {
namespace tcpsrv {

using smsc::core::network::Socket;

using smsc::inman::ICServiceAC_T;

using smsc::inman::interaction::INProtocol;
using smsc::inman::interaction::INPCommandSetAC;
using smsc::inman::interaction::INPSerializer;
using smsc::inman::interaction::ServSocketCFG;
using smsc::inman::interaction::Server;
using smsc::inman::interaction::ServerListenerITF;
using smsc::inman::interaction::SerializablePacketAC;

using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ConnectAC;


class ICSTcpServer : public ICServiceAC_T<ServSocketCFG>, public TCPServerITF,
                    ServerListenerITF, ConnectListenerITF {
private:
    struct SessionInfo {
        enum _BindType { bindSockId = 0, bindSessId } type;
        unsigned            sId;
        Connect*            conn;
        ConnServiceITF *    connSrv;
        const INPCommandSetAC * pCs;

        SessionInfo(Connect* use_conn = NULL, INPCommandSetAC * use_Cs = NULL)
            : type(bindSockId), sId(0), conn(use_conn), connSrv(0), pCs(use_Cs)
        { }

        void rlseConnManager(void)
        {
            connSrv->rlseConnManager(sId); connSrv = NULL;
        }
    };
    typedef std::map<unsigned, SessionInfo> SessionsMap;
    typedef std::map<unsigned /*sock_id*/, unsigned /*sess_id*/> SocketsMap;
    typedef std::map<INProtocol::CSId, ConnServiceITF *> ProtocolsMap;

    mutable Mutex   _sync;
    const char *    _logId; //logging prefix
    ProtocolsMap    csSrv;  //protocol servers
    SessionsMap     sessions;
    SocketsMap      sockets;
    unsigned        lastSessId;
    std::auto_ptr<ServSocketCFG> _cfg;
    std::auto_ptr<smsc::inman::interaction::Server>   tcpSrv;

    void closeSession(unsigned sockId);

protected:
    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void)
    {
        //initialize Protocol Serializer, set TCP server listener 
        INPSerializer::getInstance();
        tcpSrv.reset(new Server(_cfg.get(), logger));
        tcpSrv->addListener(this);
        return ICServiceAC::icsRcOk;
    }
    //Starts service verifying that all dependent services are started
    RCode _icsStart(void)
    {
        return tcpSrv->Start() ? ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
    }
    //Stops service
    void  _icsStop(bool do_wait = false)
    {
        _sync.Unlock();
        try { tcpSrv->Stop(do_wait ? Server::_SHUTDOWN_TMO_MS : 0);
        } catch (const std::exception & exc) {
            smsc_log_error(logger, "%s: TCP server Stop() exception: %s", _logId, exc.what());
        } catch (...) {
            smsc_log_error(logger, "%s: TCP server Stop() exception: <unknown", _logId);
        }
        _sync.Lock();
    }


    friend class smsc::inman::interaction::Server;
    // -------------------------------------
    // -- ServerListenerITF interface mthods
    // -------------------------------------
    //Listener creates its own ConnectAC implementation
    ConnectAC * onConnectOpening(Server *srv, Socket *sock);
    //Listener shouldn't delete ConnectAC !!!
    void onConnectClosing(Server *srv, ConnectAC *conn);
    void onServerShutdown(Server *srv, Server::ShutdownReason reason);

    friend class smsc::inman::interaction::Connect;
    // -------------------------------------
    // -- ConnectListenerITF interface mthods
    // -------------------------------------
    //NOTE: it's recommended to reset exception if it doesn't prevent entire Connect to function
    void onConnectError(Connect *conn, std::auto_ptr<CustomException> &p_exc);
    //NOTE: if listener takes ownership of packet, remaining listeners will not be notified.
    void onPacketReceived(Connect *conn, std::auto_ptr<SerializablePacketAC> &recv_cmd);

public:
    ICSTcpServer(std::auto_ptr<ServSocketCFG> & use_cfg,
                 const ICServicesHostITF * svc_host, Logger * use_log = NULL)
        : ICServiceAC_T<ServSocketCFG>(ICSIdent::icsIdTCPServer, svc_host, use_cfg, use_log)
        , _logId("TCPSrv"), lastSessId(0)
    {
        _cfg.reset(use_cfg.release());
        _icsState = ICServiceAC::icsStConfig;
    }
    ~ICSTcpServer()
    {
        ICSStop(true);
    }

    //Returns TCPServerITF
    void * Interface(void) const { return (TCPServerITF*)this; }

    // ----------------------------------
    // -- TCPServerITF interafce methods
    // ----------------------------------
    bool registerProtocol(const INPCommandSetAC * cmd_set,
                                    ConnServiceITF * conn_srv);
};

} //tcpsrv
} //inman
} //smsc
#endif /* __INMAN_ICS_TCPSERVER_HPP */

