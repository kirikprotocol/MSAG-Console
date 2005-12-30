static char const ident[] = "$Id$";
#include <assert.h>

#include "inman/inap/dispatcher.hpp"
#include "service.hpp"

using smsc::inman::interaction::InmanCommand;
using smsc::inman::interaction::SerializerInap;
using smsc::inman::inap::TCAPDispatcher;


namespace smsc  {
namespace inman {

Service::Service(const InService_CFG * in_cfg, Logger * uselog/* = NULL*/)
    : logger(uselog), _cfg(*in_cfg)
    , session(0), disp(0), server(0), bfs(0)
    , tcpRestartCount(0)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.Service");

    smsc_log_debug(logger, "InmanSrv: Creating ..");

    disp = TCAPDispatcher::getInstance();
    _cfg.bill.userId += 39; //adjust USER_ID to PortSS7 units id
    if (!disp->connect(_cfg.bill.userId, TCAPDispatcher::ss7CONNECTED))
        smsc_log_error(logger, "InmanSrv: EINSS7 stack unavailable!!!");
    else {
        smsc_log_debug(logger, "InmanSrv: TCAP dispatcher has connected to SS7 stack");
        session = disp->openSession(_cfg.bill.ssn, _cfg.bill.ssf_addr, _cfg.bill.scf_addr);
        if (session)
            smsc_log_debug(logger, "InmanSrv: TCAP session inited");
    }

    server = new Server(_cfg.host, _cfg.port, SerializerInap::getInstance(),
                        _cfg.bill.tcpTimeout, 10, logger);
    server->addListener(this);
    smsc_log_debug(logger, "InmanSrv: TCP server inited");

    if (_cfg.bill.cdrMode) {
        bfs = new InBillingFileStorage(_cfg.bill.billingDir, 0, logger);
        assert(bfs);
        int oldfs = bfs->RFSOpen(true);
        assert(oldfs >= 0);
        smsc_log_debug(logger, "InmanSrv: Billing storage opened%s",
                       oldfs > 0 ? ", old files rolled": "");

        if (_cfg.bill.billingInterval) { //use external storage roller
            roller = new InFileStorageRoller(bfs, (unsigned long)_cfg.bill.billingInterval);
            assert(roller);
            smsc_log_debug(logger, "InmanSrv: BillingStorage roller inited");
        }
    }
}

Service::~Service()
{
    if (running)
      stop();

    smsc_log_debug( logger, "InmanSrv: Releasing .." );

    smsc_log_debug( logger, "InmanSrv: Releasing TCAP Sessions" );
    disp->closeAllSessions();
    smsc_log_debug( logger, "InmanSrv: Disconnecting SS7 stack");
    disp->disconnect();

    if (server) {
        server->removeListener(this);
        smsc_log_debug( logger, "InmanSrv: Deleting TCP server" );
        delete server;
    }

    if (bfs) {
        smsc_log_debug( logger, "InmanSrv: Closing Billing storage");
        bfs->RFSClose();
        if (roller)
            delete roller;
        delete bfs;
    }
    smsc_log_debug( logger, "InmanSrv: Released." );
}


void Service::writeCDR(unsigned int bcId, unsigned int bilId, const CDRRecord & cdr)
{
    bfs->bill(cdr);
    smsc_log_debug(logger, "InmanSrv: CDR written for Billing[%u.%u]",
                   bcId, bilId);
}


void Service::start()
{
    smsc_log_debug(logger, "InmanSrv: Starting TCP server ..");
    server->Start();
    
    if (session) {
        smsc_log_debug(logger, "InmanSrv: Starting TCAP dispatcher ..");
        disp->Start();
    }
    if (roller) {
        smsc_log_debug(logger, "InmanSrv: Starting BillingStorage roller ..");
        roller->Start();
    }
    running = true;
    smsc_log_debug(logger, "InmanSrv: Started.");
}

void Service::stop()
{
    if (server) {
        smsc_log_debug( logger, "InmanSrv: Stopping TCP server ..");
        server->Stop();
        server->WaitFor();
    }

    smsc_log_debug( logger, "InmanSrv: Stopping TCAP dispatcher ..");
    disp->Stop();
    disp->WaitFor();

    if (roller) {
        smsc_log_debug(logger, "InmanSrv: Stopping BillingStorage roller ..");
        roller->Stop();
        roller->WaitFor();
    }
    running = false;
    smsc_log_debug(logger, "InmanSrv: Stopped.");
}

//Local point requests to end connection
void Service::onBillingConnectClosed(unsigned int connId)
{
    _mutex.Lock();
    BillingConnMap::const_iterator it = bConnects.find(connId);
    if (it != bConnects.end()) {
        BillingConnect* bcon = (*it).second;
        bConnects.erase(connId);
        _mutex.Unlock();
        delete bcon;
        smsc_log_debug(logger, "InmanSrv: BillingConnect[0x%X] closed", connId);
    } else {
        _mutex.Unlock();
        smsc_log_warn(logger, "InmanSrv: attempt to close unknown connect[0x%X]", connId);
    }
}
/* -------------------------------------------------------------------------- *
 * ServerListener interface implementation:
 * -------------------------------------------------------------------------- */
void Service::onConnectOpened(Server* srv, Connect* conn)
{
    assert(conn);
    conn->setConnectFormat(Connect::frmLengthPrefixed);
    BillingConnect *bcon = new BillingConnect(&_cfg.bill, session, conn, this, logger);
    if (bcon) {
        _mutex.Lock();
        bConnects.insert(BillingConnMap::value_type(conn->getSocketId(), bcon));
        conn->addListener(bcon);
        _mutex.Unlock();
        smsc_log_debug(logger, "InmanSrv: New BillingConnect[%u] created",
                       conn->getSocketId());
    }
}

//Remote point ends connection
void Service::onConnectClosing(Server* srv, Connect* conn)
{
    assert(conn);
    unsigned int connId = (unsigned int)conn->getSocketId();
    _mutex.Lock();
    BillingConnMap::const_iterator it = bConnects.find(connId);
    if (it != bConnects.end()) {
        BillingConnect *bcon = (*it).second;
        conn->removeListener(bcon);
        bConnects.erase(connId);
        _mutex.Unlock();

        delete bcon;
        smsc_log_debug(logger, "InmanSrv: BillingConnect[%u] closed", connId);
    } else {
        _mutex.Unlock();
        smsc_log_warn(logger, "InmanSrv: attempt to close unknown connect[%u]", connId);
    }
}

void Service::onServerShutdown(Server* srv, Server::ShutdownReason reason)
{
    smsc_log_debug(logger, "InmanSrv: TCP server shutdowned, reason %d", reason);

    if (reason != Server::srvStopped) { //try to restart
        srv->removeListener(this);
        delete srv;
        server = NULL;

        if (++tcpRestartCount <= INMAN_TCP_RESTART_ATTEMPTS) {
            smsc_log_debug(logger, "InmanSrv: Restarting TCP server ..");
            try {
                server = new Server(_cfg.host, _cfg.port, SerializerInap::getInstance(),
                                    _cfg.bill.tcpTimeout, 10, logger);
                server->addListener(this);
                smsc_log_debug(logger, "InmanSrv: TCP server inited");
                server->Start();
            } catch (CustomException & exc) {
                smsc_log_error(logger, "InmanSrv: TCP server restart failure: %s",
                               exc.what());
                ++tcpRestartCount;
                throw CustomException("InmanSrv: TCP server restart failure",
                                      tcpRestartCount, exc.what());
            }
        } else {
            throw CustomException("InmanSrv: TCP server continual failure, exiting.",
                                  tcpRestartCount);
        }
    }
}

} // namespace inmgr
} // namespace smsc
