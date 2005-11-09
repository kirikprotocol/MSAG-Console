#ident "$Id$"

#ifndef __SMSC_INMAN_SERVICE__
#define __SMSC_INMAN_SERVICE__

#include <map>

#include "logger/Logger.h"
#include "inman/inap/dispatcher.hpp"
#include "inman/interaction/server.hpp"
#include "inman/inap/session.hpp"

using smsc::inman::inap::Dispatcher;
using smsc::inman::inap::Session;
using smsc::inman::interaction::Server;
using smsc::inman::interaction::ServerListener;
using smsc::inman::interaction::ConnectListener;

namespace smsc  {
namespace inman {

class Billing;
class Service : public ServerListener, ConnectListener
{
    typedef std::map<unsigned int, Billing*> BillingMap;

public:

    Service( const char* ssf_addr, const char* scf_addr, const char* host, int port, int SSN);
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
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_SERVICE__ */

