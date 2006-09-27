#ident "$Id$"

#ifndef __SMSC_INMAN_SERVICE__
#define __SMSC_INMAN_SERVICE__

#include <map>

#include "inman/comp/acdefs.hpp"
using smsc::ac::ACOID;

#include "inman/inap/dispatcher.hpp"
using smsc::inman::inap::TCAPDispatcher;

#include "inman/interaction/server.hpp"
using smsc::inman::interaction::ServSocketCFG;
using smsc::inman::interaction::Server;
using smsc::inman::interaction::ServerListener;

#include "inman/billing.hpp"
using smsc::inman::BillingCFG;
using smsc::inman::BillingConnect;
using smsc::inman::cache::AbonentCacheCFG;
using smsc::inman::cache::AbonentCache;
using smsc::inman::filestore::InFileStorageRoller;

#include "inman/abprov/IAPLoader.hpp"
using smsc::inman::iaprvd::IAProviderCreatorITF;

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

class Service : public ServerListener {
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
    typedef std::map<unsigned int, BillingConnect*> BillingConnMap;

    Mutex           _mutex;
    BillingConnMap  bConnects;
    Logger*         logger;
    SSNSession*     ssnSess;    //
    TCAPDispatcher* disp;
    Server*         server;
    volatile bool   running;
    InService_CFG   _cfg;
    InFileStorageRoller * roller;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_SERVICE__ */

