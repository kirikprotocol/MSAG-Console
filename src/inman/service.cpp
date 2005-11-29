static char const ident[] = "$Id$";
#include <assert.h>

#include "inman/inap/infactory.hpp"
#include "service.hpp"

using smsc::inman::interaction::InmanCommand;
using smsc::inman::interaction::SerializerInap;
using smsc::inman::inap::InSessionFactory;


namespace smsc  {
namespace inman {

Service::Service(const InService_CFG * in_cfg, Logger * uselog/* = NULL*/)
    : logger(uselog), _cfg(*in_cfg)
    , session(0), dispatcher(0), server(0), bfs(0)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.Service");

    smsc_log_debug(logger, "InmanSrv: Creating ..");

    InSessionFactory* factory = InSessionFactory::getInstance();
    assert( factory );

    dispatcher = new Dispatcher();
    smsc_log_debug(logger, "InmanSrv: TCAP dispatcher inited");

    server = new Server(_cfg.host, _cfg.port, SerializerInap::getInstance());
    server->addListener(this);
    smsc_log_debug(logger, "InmanSrv: TCP server inited");

    if (_cfg.cdrMode) {
        bfs = new InBillingFileStorage(_cfg.billingDir, 0, logger);
        assert(bfs);
        int oldfs = bfs->RFSOpen(true);
        assert(oldfs >= 0);
        smsc_log_debug(logger, "InmanSrv: Billing storage opened%s",
                       oldfs > 0 ? ", old files rolled": "");

        if (_cfg.billingInterval) { //use external storage roller
            roller = new InFileStorageRoller(bfs, (unsigned long)_cfg.billingInterval);
            assert(roller);
            smsc_log_debug(logger, "InmanSrv: BillingStorage roller inited");
        }
    }

    session = factory->openSession(_cfg.ssn, _cfg.ssf_addr, _cfg.scf_addr);
    assert(session);
    smsc_log_debug(logger, "InmanSrv: TCAP session inited");

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

    if (bfs) {
        smsc_log_debug( logger, "InmanSrv: Close Billing storage");
        bfs->RFSClose();
        if (roller)
            delete roller;
        delete bfs;
    }
}


void Service::start()
{
    smsc_log_debug(logger, "InmanSrv: Starting TCAP dispatcher");
    dispatcher->Start();

    smsc_log_debug(logger, "InmanSrv: Starting TCP server");
    server->Start();

    if (roller) {
        smsc_log_debug(logger, "InmanSrv: Starting BillingStorage roller");
        roller->Start();
    }
    
    running = true;
    smsc_log_debug(logger, "InmanSrv: Started.");
}

void Service::stop()
{
    smsc_log_debug( logger, "InmanSrv: Stopping TCP server");
    server->Stop();
    server->WaitFor();

    smsc_log_debug( logger, "InmanSrv: Stopping TCAP dispatcher");
    dispatcher->Stop();
    dispatcher->WaitFor();

    if (roller) {
        smsc_log_debug(logger, "InmanSrv: Stopping BillingStorage roller");
        roller->Stop();
        roller->WaitFor();
    }
    running = false;
}

void Service::billingFinished(Billing* bill)
{
    assert( bill );
    unsigned int billId = bill->getId();

    BillingMap::iterator it = workers.find(billId);
    if (it == workers.end())
        smsc_log_error(logger, "InmanSrv: Attempt to free unregistered Billing, id: 0x%X", billId);
    else {
        if (_cfg.cdrMode && bill->BillComplete()) {
            if ((_cfg.cdrMode == InService_CFG::CDR_ALL)
                || ((_cfg.cdrMode == InService_CFG::CDR_PREPAID)
                    && (bill->getBillingType() == Billing::billPrepaid))
                || ((_cfg.cdrMode == InService_CFG::CDR_POSTPAID)
                    && (bill->getBillingType() == Billing::billPostpaid)))
                bfs->bill(bill->getCDRRecord());
        }
        workers.erase(billId);
    }
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
        bill = new Billing(this, dlgId, session, conn, _cfg.billMode);
        workers.insert(BillingMap::value_type(dlgId, bill));
    } else
        bill = (*it).second;
    bill->handleCommand(cmd);
}


} // namespace inmgr
} // namespace smsc
