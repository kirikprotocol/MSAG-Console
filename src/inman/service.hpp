#ident "$Id$"

#ifndef __SMSC_INMAN_SERVICE__
#define __SMSC_INMAN_SERVICE__

#include <map>

#include "inman/inap/dispatcher.hpp"
using smsc::inman::inap::TCAPDispatcher;

#include "inman/interaction/server.hpp"
using smsc::inman::interaction::ServSocketCFG;
using smsc::inman::interaction::Server;
using smsc::inman::interaction::ServerListenerITF;
using smsc::inman::interaction::ConnectAC;

#include "inman/interaction/connect.hpp"
using smsc::inman::interaction::Connect;

#include "inman/BillingManager.hpp"
using smsc::inman::BillingCFG;
using smsc::inman::filestore::InFileStorageRoller;

#include "inman/incache.hpp"
using smsc::inman::cache::AbonentCacheCFG;

namespace smsc  {
namespace inman {

struct InService_CFG {
    ServSocketCFG       sock;
    BillingCFG          bill;
    AbonentCacheCFG     cachePrm;
    AbonentPolicies     abPolicies;

    InService_CFG()
    {
        bill.policies = &abPolicies;
        sock.port = sock.maxConn = sock.timeout = 0;
    }
};

typedef struct {
    enum _BindType {
        bindSockId = 0, bindSessId
    }          type;
    unsigned    sId;
    Connect*    conn;
    ConnectManagerAC * hdl;
} SessionInfo;

class Service : public ServerListenerITF, public ConnectListenerITF {
public:
    Service(const InService_CFG * in_cfg, Logger * uselog = NULL);
    virtual ~Service();

    bool start();
    void stop();

protected:
    friend class smsc::inman::interaction::Server;
    //-- ServerListenerITF interface methods
    ConnectAC * onConnectOpening(Server* srv, Socket* sock);
    void onConnectClosing(Server* srv, ConnectAC* conn);
    void onServerShutdown(Server* srv, Server::ShutdownReason reason);

    friend class smsc::inman::interaction::Connect;
    //-- ConnectListenerITF methods
    void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
        /*throw (std::exception)*/;
    //NOTE: it's recommended to reset exception if it doesn't prevent entire Connect to function
    void onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc);

private:
    typedef std::map<unsigned, SessionInfo> SessionsMap;
    typedef std::map<unsigned /*sock_id*/, unsigned /*sess_id*/> SocketsMap;

    void closeSession(unsigned sockId);

    Mutex           _mutex;
    Logger*         logger;
    TCAPDispatcher* disp;
    Server*         server;
    volatile bool   running;
    InService_CFG   _cfg;
    InFileStorageRoller * roller;

    SessionsMap     sessions;
    SocketsMap      sockets;
    unsigned        lastSessId;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_SERVICE__ */

