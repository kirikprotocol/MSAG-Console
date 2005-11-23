#ident "$Id$"

#ifndef __SMSC_INMAN_SERVICE__
#define __SMSC_INMAN_SERVICE__

#include <map>

#include "logger/Logger.h"
#include "inman/inap/dispatcher.hpp"
#include "inman/interaction/server.hpp"
#include "inman/inap/session.hpp"
#include "inman/storage/FileStorages.hpp"

using smsc::inman::inap::Dispatcher;
using smsc::inman::inap::Session;
using smsc::inman::interaction::Server;
using smsc::inman::interaction::ServerListener;
using smsc::inman::interaction::ConnectListener;
using smsc::inman::filestore::InBillingFileStorage;

namespace smsc  {
namespace inman {

struct InService_CFG {
    typedef enum { BILL_NONE = 0, BILL_ALL = 1, BILL_POSTPAID = 2 } BILL_MODE;

    const char*     ssf_addr;
    const char*     scf_addr;
    int             ssn;
    const char*     host;
    int             port;
    BILL_MODE       billMode;
    const char *    billingDir;      //location to store CDR files
    long            billingInterval; //rolling interval for CDR files
};

class Billing;
class Service : public ServerListener, ConnectListener
{
    typedef std::map<unsigned int, Billing*> BillingMap;

public:
    Service(const InService_CFG * in_cfg, Logger * uselog = NULL);
    virtual ~Service();

    //ServerListener interface
    virtual void onConnectOpened(Server*, Connect*);
    virtual void onConnectClosed(Server*, Connect*);

    //ConnectListener interface
    virtual void onCommandReceived(Connect*, SerializableObject*);

    virtual void billingFinished(Billing* bill);

    virtual void start();
    virtual void stop();

private:
    BillingMap  workers;
    Logger*     logger;
    Session*    session;
    Dispatcher* dispatcher;
    Server*     server;
    bool        running;
    InService_CFG _cfg;
    InBillingFileStorage*    bfs;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_SERVICE__ */

