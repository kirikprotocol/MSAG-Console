static char const ident[] = "$Id$";
#include <assert.h>

#include "inman/inap/infactory.hpp"
#include "service.hpp"
#include "billing.hpp"

using smsc::inman::interaction::InmanCommand;
using smsc::inman::interaction::SerializerInap;
using smsc::inman::inap::InSessionFactory;


namespace smsc  {
namespace inman {

Service::Service( const char* ssf_addr, const char* scf_addr, const char* host, int port, int SSN)
        : logger( Logger::getInstance("smsc.inman.Service") )
        , session( 0 )
        , dispatcher( 0 )
        , server( 0 )
{
    smsc_log_debug(logger, "InmanSrv: Creating ..");

    InSessionFactory* factory = InSessionFactory::getInstance();
    assert( factory );

    dispatcher = new Dispatcher();
    smsc_log_debug(logger, "InmanSrv: TCAP dispatcher inited");

    server = new Server( host, port, SerializerInap::getInstance() );
    server->addListener( this );
    smsc_log_debug(logger, "InmanSrv: TCP server inited");

    session = factory->openSession(SSN, ssf_addr, scf_addr );
    assert( session );
}

Service::~Service()
{
    if (running)
      stop();

    smsc_log_debug( logger, "InmanSrv: Releasing .." );
    InSessionFactory* factory = InSessionFactory::getInstance();

    smsc_log_debug( logger, "InmanSrv: ReleaseSession" );
    factory->closeSession( session );

    server->removeListener( this );
    smsc_log_debug( logger, "InmanSrv: Delete TCP server" );
    delete server;

    smsc_log_debug( logger, "InmanSrv: Delete TCAP dispatcher");
    delete dispatcher;
}


void Service::start()
{
    smsc_log_debug(logger, "InmanSrv: Starting TCAP dispatcher");
    dispatcher->Start();

    smsc_log_debug(logger, "InmanSrv: Starting TCP server");
    server->Start();
    running = true;
}

void Service::stop()
{
    smsc_log_debug( logger, "InmanSrv: Stopping TCP server");
    server->Stop();
    server->WaitFor();

    smsc_log_debug( logger, "InmanSrv: Stopping TCAP dispatcher");
    dispatcher->Stop();
    dispatcher->WaitFor();
    running = false;
}

void Service::billingFinished(Billing* bill)
{
    assert( bill );
    unsigned int billId = bill->getId();

    BillingMap::iterator it = workers.find(billId);
    if (it == workers.end())
        smsc_log_error(logger, "InmanSrv: Attempt to free unregistered Billing, id: 0x%X", billId);
    else
        workers.erase(billId);
    delete bill;
    smsc_log_debug(logger, "InmanSrv: Billing deleted, id: 0x%X", billId);
}

/* -------------------------------------------------------------------------- *
 * ServerListener interface implementation:
 * -------------------------------------------------------------------------- */
void Service::onConnectOpened(Server*, Connect* connect)
{
    assert( connect );
    smsc_log_debug(logger, "InmanSrv: New connection opened");
    connect->addListener( this );
    connect->setPipeFormat(ObjectPipe::frmLengthPrefixed);
}

void Service::onConnectClosed(Server*, Connect* connect)
{
    assert( connect );
    smsc_log_debug( logger, "InmanSrv: Connection closed" );
    connect->removeListener( this );
}

/* -------------------------------------------------------------------------- *
 * ConnectListener interface implementation:
 * -------------------------------------------------------------------------- */
void Service::onCommandReceived(Connect* conn, SerializableObject* recvCmd)
{
    InmanCommand* cmd = static_cast<InmanCommand*>(recvCmd);
    assert( cmd );

    unsigned int dlgId = cmd->getDialogId();
    smsc_log_debug(logger, "InmanSrv: Command 0x%X for Billing 0x%X received",
                   cmd->getObjectId(), dlgId);

    Billing* bill;
    BillingMap::iterator it = workers.find(dlgId);
    if (it == workers.end()) {
        bill = new Billing(this, dlgId, session, conn);
        workers.insert(BillingMap::value_type(dlgId, bill));
    } else
        bill = (*it).second;
    bill->handleCommand(cmd);
}


} // namespace inmgr
} // namespace smsc
