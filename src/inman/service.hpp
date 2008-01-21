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
using smsc::inman::interaction::INPCommandSetAC;

#include "inman/AbntDetManager.hpp"
using smsc::inman::AbonentDetectorCFG;

#include "inman/common/TaskScheduler.hpp"
using smsc::util::TaskSchedulerITF;
using smsc::util::TaskSchedulerAC;

#include "inman/incache/InCacheMT.hpp"
using smsc::inman::cache::AbonentCacheCFG;
using smsc::inman::cache::AbonentCacheMTR;

namespace smsc  {
namespace inman {

struct InService_CFG {
public:
    ServSocketCFG       sock;
    BillingCFG          bill;
    AbonentDetectorCFG  dtcr;
    AbonentCacheCFG     cachePrm;
    AbonentPolicies     abPolicies;
    IAPrvdsRegistry     abPrvds;

    InService_CFG()
    {
        dtcr.policies = bill.policies = &abPolicies;
        sock.port = sock.maxConn = sock.timeout = 0;
    }
};

struct SessionInfo {
    enum _BindType { bindSockId = 0, bindSessId } type;
    unsigned            sId;
    Connect*            conn;
    ConnectManagerAC *  hdl;
    INPCommandSetAC *   pCs;

    SessionInfo(INPCommandSetAC * use_Cs = NULL, Connect* use_conn = NULL)
        : type(bindSockId), sId(0), conn(use_conn), hdl(0), pCs(use_Cs)
    { }
};

class Service : public ServerListenerITF,
                public ConnectListenerITF, public TaskSchedulerFactoryITF {
public:
    //Service takes ownership of in_cfg
    Service(InService_CFG * in_cfg, Logger * uselog = NULL);
    virtual ~Service();

    bool start();
    void stop();

    //-- TaskSchedulerFactoryITF interface methods: --//
    TaskSchedulerITF * getScheduler(TaskSchedulerITF::SchedulerType sched_type);

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
    typedef std::pair<TaskSchedulerITF::SchedulerType, TaskSchedulerAC*> TSchedRCD;
    typedef std::list<TSchedRCD> TSchedList;

    void closeSession(unsigned sockId);

    Mutex           _mutex;
    const char *    _logId; //logging prefix
    Logger*         logger;
    TCAPDispatcher* disp;
    Server*         server;
    volatile bool   running;
    InService_CFG*  _cfg;
    InFileStorageRoller * roller;
    AbonentCacheMTR * abCache;
    SessionsMap     sessions;
    SocketsMap      sockets;
    unsigned        lastSessId;
    TSchedList      tschedList;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_SERVICE__ */

