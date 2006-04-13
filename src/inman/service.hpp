#ident "$Id$"

#ifndef __SMSC_INMAN_SERVICE__
#define __SMSC_INMAN_SERVICE__

#include <map>

#include "inman/comp/acdefs.hpp"
using smsc::ac::ACOID;

#include "inman/inap/dispatcher.hpp"
#include "inman/interaction/server.hpp"
#include "inman/billing.hpp"

using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::interaction::ServSocketCFG;
using smsc::inman::interaction::Server;
using smsc::inman::interaction::ServerListener;
using smsc::inman::filestore::InFileStorageRoller;
using smsc::inman::BillingCFG;
using smsc::inman::BillingConnect;
using smsc::inman::sync::TimeWatcher;
using smsc::inman::cache::AbonentCacheCFG;
using smsc::inman::cache::AbonentCache;

namespace smsc  {
namespace inman {

struct InService_CFG {
    ServSocketCFG   sock;
    BillingCFG      bill;
    AbonentCacheCFG cachePrm;
};

class Service : public ServerListener
{
    typedef std::map<unsigned int, BillingConnect*> BillingConnMap;

public:
    Service(const InService_CFG * in_cfg, Logger * uselog = NULL);
    virtual ~Service();

    bool start();
    void stop();
    void onBillingConnectClosed(unsigned int connId);
    
    //ServerListener interface
    void onConnectOpened(Server* srv, Connect* conn);
    void onConnectClosing(Server* srv, Connect* conn);
    void onServerShutdown(Server* srv, Server::ShutdownReason reason);

private:
    Mutex           _mutex;
    BillingConnMap  bConnects;
    Logger*         logger;
    SSNSession*     session;
    TCAPDispatcher* disp;
    Server*         server;
    volatile bool   running;
    InService_CFG   _cfg;
    InFileStorageRoller *    roller;
    TimeWatcher*    tmWatcher;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_SERVICE__ */

