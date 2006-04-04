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
    : logger(uselog), _cfg(*in_cfg), session(0), disp(0)
    , server(0), tmWatcher(NULL)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.Service");

    smsc_log_debug(logger, "InmanSrv: Creating ..");

    disp = TCAPDispatcher::getInstance();
    _cfg.bill.userId += 39; //adjust USER_ID to PortSS7 units id
    if (!disp->connect(_cfg.bill.userId, _cfg.bill.ssn))
        smsc_log_error(logger, "InmanSrv: EINSS7 stack unavailable!!!");
    else {
        smsc_log_debug(logger, "InmanSrv: TCAP dispatcher has connected to SS7 stack");
        session = disp->openSession(_cfg.bill.ssn, _cfg.bill.ssf_addr,
                                    _cfg.bill.scf_addr, id_ac_cap3_sms_AC);
        if (session)
            smsc_log_debug(logger, "InmanSrv: TCAP session inited");
    }

    server = new Server(&_cfg.sock, SerializerInap::getInstance(), logger);
    assert(server);
    server->addListener(this);
    smsc_log_debug(logger, "InmanSrv: TCP server inited");

    if (_cfg.bill.cdrMode) {
        _cfg.bill.bfs = new InBillingFileStorage(_cfg.bill.cdrDir, 0, logger);
        assert(_cfg.bill.bfs);
        int oldfs = _cfg.bill.bfs->RFSOpen(true);
        assert(oldfs >= 0);
        smsc_log_debug(logger, "InmanSrv: Billing storage opened%s",
                       oldfs > 0 ? ", old files rolled": "");

        if (_cfg.bill.cdrInterval) { //use external storage roller
            roller = new InFileStorageRoller(_cfg.bill.bfs,
                                             (unsigned long)_cfg.bill.cdrInterval);
            assert(roller);
            smsc_log_debug(logger, "InmanSrv: BillingStorage roller inited");
        }
    } else
        _cfg.bill.bfs = NULL;
    tmWatcher = new TimeWatcher(logger);
    assert(tmWatcher);
    smsc_log_debug(logger, "InmanSrv: TimeWatcher inited");

    _cfg.bill.abCache = new AbonentCache(&_cfg.cachePrm, logger);
    assert(_cfg.bill.abCache);
    if (_cfg.bill.abProvider)
        _cfg.bill.abProvider->bindCache(_cfg.bill.abCache);
    smsc_log_debug(logger, "InmanSrv: AbonentCache inited");
}

Service::~Service()
{
    smsc_log_debug(logger, "InmanSrv: Releasing ..");
    if (running)
      stop();

    smsc_log_debug(logger, "InmanSrv: Disconnecting SS7 stack ..");
    disp->disconnect();

    if (server) {
        server->removeListener(this);
        smsc_log_debug( logger, "InmanSrv: Deleting TCP server ..");
        delete server;
    }
    if (_cfg.bill.bfs) {
        smsc_log_debug(logger, "InmanSrv: Closing Billing storage ..");
        _cfg.bill.bfs->RFSClose();
        if (roller)
            delete roller;
        delete _cfg.bill.bfs;
    }
    if (tmWatcher) {
        smsc_log_debug(logger, "InmanSrv: Deleting TimeWatcher ..");
        delete tmWatcher;
    }
    if (_cfg.bill.abProvider) {
        smsc_log_debug(logger, "InmanSrv: Deleting AbonentsProvider ..");
        delete _cfg.bill.abProvider;
    }
    if (_cfg.bill.abCache) {
        smsc_log_debug(logger, "InmanSrv: Closing AbonentsCache ..");
        delete _cfg.bill.abCache;
    }
    smsc_log_debug( logger, "InmanSrv: Released." );
}

bool Service::start()
{
    smsc_log_debug(logger, "InmanSrv: Starting TimeWatcher ..");
    tmWatcher->Start();
    if (!tmWatcher->isRunning())
        return false;

    smsc_log_debug(logger, "InmanSrv: Starting TCP server ..");
    if (!server->Start())
        return false;
    
    if (roller) {
        smsc_log_debug(logger, "InmanSrv: Starting BillingStorage roller ..");
        roller->Start();
    }
    running = true;
    smsc_log_debug(logger, "InmanSrv: Started.");
    return running;
}

void Service::stop()
{
    if (server) {
        smsc_log_debug( logger, "InmanSrv: Stopping TCP server ..");
        server->Stop();
    }

    smsc_log_debug(logger, "InmanSrv: Stopping TimeWatcher ..");
    tmWatcher->Stop();
    tmWatcher->WaitFor();

    smsc_log_debug( logger, "InmanSrv: Stopping TCAP dispatcher ..");
    disp->Stop();

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
    BillingConnect *bcon = new BillingConnect(&_cfg.bill, session, conn, 
                                              tmWatcher, logger);
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
        smsc_log_info(logger, "InmanSrv: BillingConnect[%u] closed", connId);
    } else {
        _mutex.Unlock();
        smsc_log_warn(logger, "InmanSrv: attempt to close unknown connect[%u]", connId);
    }
}
//throws CustomException
void Service::onServerShutdown(Server* srv, Server::ShutdownReason reason)
{
    smsc_log_debug(logger, "InmanSrv: TCP server shutdowning, reason %d", reason);
    srv->removeListener(this);
    delete srv;
    server = NULL;

    if (reason != Server::srvStopped) { //abnormal shutdown
        throw CustomException("InmanSrv: TCP server fatal failure, exiting.");
    }
}

} // namespace inmgr
} // namespace smsc
