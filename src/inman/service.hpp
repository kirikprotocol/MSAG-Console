#ident "$Id$"

#ifndef __SMSC_INMAN_SERVICE__
#define __SMSC_INMAN_SERVICE__

#include <map>

#include "inman/inap/dispatcher.hpp"
#include "inman/interaction/server.hpp"
#include "inman/inap/session.hpp"
#include "inman/storage/FileStorages.hpp"
#include "inman/billing.hpp"

using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::inap::Session;
using smsc::inman::interaction::Server;
using smsc::inman::interaction::ServerListener;
using smsc::inman::filestore::InBillingFileStorage;
using smsc::inman::filestore::InFileStorageRoller;
using smsc::inman::BillingCFG;
using smsc::inman::BillingConnect;

#define INMAN_TCP_RESTART_ATTEMPTS  2

namespace smsc  {
namespace inman {

struct InService_CFG {
    const char*     host;
    int             port;
    BillingCFG      bill;
};

class Service : public ServerListener
{
    typedef std::map<unsigned int, BillingConnect*> BillingConnMap;

public:
    Service(const InService_CFG * in_cfg, Logger * uselog = NULL);
    virtual ~Service();

    void start();
    void stop();
    void writeCDR(unsigned int bcId, unsigned int bilId, const CDRRecord & cdr);

    void onBillingConnectClosed(unsigned int connId);
    
    //ServerListener interface
    void onConnectOpened(Server* srv, Connect* conn);
    void onConnectClosing(Server* srv, Connect* conn);
    void onServerShutdown(Server* srv, Server::ShutdownReason reason);

private:
    Mutex           _mutex;
    BillingConnMap  bConnects;
    Logger*         logger;
    Session*        session;
    TCAPDispatcher* disp;
    Server*         server;
    volatile bool   running;
    InService_CFG   _cfg;
    InBillingFileStorage*    bfs;
    InFileStorageRoller *    roller;
    unsigned        tcpRestartCount;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_SERVICE__ */

