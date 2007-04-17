#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */
#include <assert.h>

#include "inman/inap/dispatcher.hpp"
using smsc::inman::inap::TCAPDispatcher;

#include "inman/service.hpp"
using smsc::inman::interaction::INPSerializer;

#include "inman/AbntDetManager.hpp"

namespace smsc  {
namespace inman {
/* ************************************************************************** *
 * class Service implementation:
 * ************************************************************************** */
Service::Service(InService_CFG * in_cfg, Logger * uselog/* = NULL*/)
    : logger(uselog), _cfg(in_cfg), disp(0), server(0), lastSessId(0), abCache(0)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.Service");

    smsc_log_debug(logger, "InmanSrv: Creating ..");

    if (_cfg->bill.billMode != smsc::inman::BILL_NONE) {
        disp = TCAPDispatcher::getInstance();
        _cfg->bill.ss7.userId += 39; //adjust USER_ID to PortSS7 units id
        if (!disp->connect(_cfg->bill.ss7.userId))
            smsc_log_error(logger, "InmanSrv: EINSS7 stack unavailable!!!");
        else {
            smsc_log_debug(logger, "InmanSrv: TCAP dispatcher has connected to SS7 stack");
            if (!disp->openSSN(_cfg->bill.ss7.own_ssn, _cfg->bill.ss7.maxDlgId))
                smsc_log_error(logger, "InmanSrv: SSN[%u] unavailable!!!", _cfg->bill.ss7.own_ssn);
        }
    }                                         

    _cfg->bill.abCache = (abCache = new AbonentCacheMTR(&_cfg->cachePrm, logger));
    assert(_cfg->bill.abCache);
    smsc_log_debug(logger, "InmanSrv: AbonentCache inited");

    //initialize IAProviders for defined policies
    for (AbonentPolicies::iterator pit = _cfg->abPolicies.begin(); 
                                    pit != _cfg->abPolicies.end(); pit++) {
        AbonentPolicy *pol = *pit;
        pol->bindCache(_cfg->bill.abCache);
        pol->getIAProvider(logger);
    }

    INPSerializer::getInstance();
    server = new Server(&_cfg->sock, logger);
    assert(server);
    server->addListener(this);
    smsc_log_debug(logger, "InmanSrv: TCP server inited");

    if (_cfg->bill.cdrMode) {
        _cfg->bill.bfs = new InBillingFileStorage(_cfg->bill.cdrDir, 0, logger);
        assert(_cfg->bill.bfs);
        int oldfs = _cfg->bill.bfs->RFSOpen(true);
        assert(oldfs >= 0);
        smsc_log_debug(logger, "InmanSrv: Billing storage opened%s",
                       oldfs > 0 ? ", old files rolled": "");

        if (_cfg->bill.cdrInterval) { //use external storage roller
            roller = new InFileStorageRoller(_cfg->bill.bfs,
                                             (unsigned long)_cfg->bill.cdrInterval);
            assert(roller);
            smsc_log_debug(logger, "InmanSrv: BillingStorage roller inited");
        }
    } else
        _cfg->bill.bfs = NULL;

    _cfg->bill.tmWatcher = new TimeWatcher(logger);
    assert(_cfg->bill.tmWatcher);
    smsc_log_debug(logger, "InmanSrv: TimeWatcher inited");
}

Service::~Service()
{
    smsc_log_debug(logger, "InmanSrv: Releasing ..");
    if (running)
      stop();

    if (disp) {
        smsc_log_debug(logger, "InmanSrv: Disconnecting SS7 stack ..");
        disp->disconnect();
    }

    if (server) {
        server->removeListener(this);
        smsc_log_debug( logger, "InmanSrv: Deleting TCP server ..");
        delete server;
    }
    if (_cfg->bill.bfs) {
        smsc_log_debug(logger, "InmanSrv: Closing Billing storage ..");
        _cfg->bill.bfs->RFSClose();
        if (roller)
            delete roller;
        delete _cfg->bill.bfs;
    }
    if (_cfg->bill.tmWatcher) {
        smsc_log_debug(logger, "InmanSrv: Deleting TimeWatcher ..");
        delete _cfg->bill.tmWatcher;
    }
    if (abCache) {
        smsc_log_debug(logger, "InmanSrv: Closing AbonentsCache ..");
        delete abCache;
    }
    delete _cfg;
    smsc_log_debug(logger, "InmanSrv: Released.");
}

bool Service::start()
{
    smsc_log_debug(logger, "InmanSrv: Starting TimeWatcher ..");
    _cfg->bill.tmWatcher->Start();
    if (!_cfg->bill.tmWatcher->isRunning())
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
    _cfg->bill.tmWatcher->Stop();
    _cfg->bill.tmWatcher->WaitFor();

    if (disp) {
        smsc_log_debug( logger, "InmanSrv: Stopping TCAP dispatcher ..");
        disp->Stop();
    }

    if (roller) {
        smsc_log_debug(logger, "InmanSrv: Stopping BillingStorage roller ..");
        roller->Stop(true);
    }

    running = false;
    smsc_log_debug(logger, "InmanSrv: Stopped.");
}

void Service::closeSession(unsigned sockId)
{
    ConnectManagerAC * pCm = NULL;
    {
        MutexGuard grd(_mutex);
        SocketsMap::iterator it = sockets.find(sockId);
        if (it == sockets.end() || !(*it).second)
            return;
        unsigned sesId = (*it).second;
        sockets.erase(it);

        SessionsMap::iterator sit = sessions.find(sesId);
        if (sit != sessions.end()) {
            SessionInfo & sess = (*sit).second;
            pCm = sess.hdl;
            smsc_log_info(logger, "InmanSrv: closing %s session[%u] on Connect[%u]",
                            sess.pCs->CsName(), sess.sId, sockId);
            sessions.erase(sit);
        }
    }
    if (pCm)
        delete pCm;
    return;
}

/* -------------------------------------------------------------------------- *
 * ServerListener interface implementation:
 * -------------------------------------------------------------------------- */
ConnectAC * Service::onConnectOpening(Server* srv, Socket* sock)
{
    Connect * conn = new Connect(sock, INPSerializer::getInstance());
    conn->addListener(this);
//    smsc_log_debug(logger, "InmanSrv: New Connect[%u] inited", conn->getId());
    return conn;
}

//Remote point ends connection
void Service::onConnectClosing(Server* srv, ConnectAC* conn)
{
    closeSession(conn->getId());
}

//throws CustomException
void Service::onServerShutdown(Server* srv, Server::ShutdownReason reason)
{
    smsc_log_debug(logger, "InmanSrv: TCP server shutdowning, reason %d", reason);
    srv->removeListener(this);

    if (reason != Server::srvStopped) { //abnormal shutdown
        throw CustomException("InmanSrv: TCP server fatal failure, exiting.");
    }
}

/* -------------------------------------------------------------------------- *
 * ConnectListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//Creates/Restores session (creates/rebinds ConnectManager)
void Service::onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
                /*throw(std::exception)*/
{
    INPPacketAC* pck = static_cast<INPPacketAC*>(recv_cmd.get());
    INPCommandSetAC * pCs = pck->pCmd()->commandSet();
    //NOTE: session restoration is not supported for now, so just create new session
    //NOTE: only commands with HDR_DIALOG is supported for now (bindSockId session type)
    SessionInfo  newSess(pCs, conn);

    switch (pCs->CsId()) {
    case smsc::inman::interaction::csBilling: {
        newSess.hdl = new BillingManager(&_cfg->bill, newSess.sId, conn, logger);
    } break;

    case smsc::inman::interaction::csAbntContract: {
        AbonentDetectorCFG sCfg;
        sCfg.tmWatcher = _cfg->bill.tmWatcher;
        sCfg.abCache = _cfg->bill.abCache;
        sCfg.policies = _cfg->bill.policies;
        sCfg.abtTimeout = _cfg->bill.abtTimeout;
        sCfg.maxRequests = _cfg->bill.maxBilling;
        sCfg.ss7 = _cfg->bill.ss7;

        newSess.hdl = new AbntDetectorManager(&sCfg, newSess.sId, conn, logger);
    } break;

    default: //force connect closing by TCPSrv
        throw CustomException("InmanSrv: unsupported CommandSet: %s (%u)",
                              pCs->CsName(), pCs->CsId());
    }
    _mutex.Lock();
    newSess.sId = ++lastSessId;
    sockets.insert(SocketsMap::value_type(conn->getId(), newSess.sId));
    sessions.insert(SessionsMap::value_type(newSess.sId, newSess));
    _mutex.Unlock();
    smsc_log_info(logger, "InmanSrv: New %s session[%u] on Connect[%u] created",
                  pCs->CsName(), newSess.sId, conn->getId());

    conn->folowUp(this, newSess.hdl);
    conn->removeListener(this);
}

//NOTE: session restoration is not supported for now, so just delete session
void Service::onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc)
{ 
    conn->removeListener(this);
    closeSession(conn->getId());
}

} // namespace inmgr
} // namespace smsc
