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
    , _logId("InmanSrv")
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.Service");

    smsc_log_debug(logger, "%s: Creating ..", _logId);

    if (_cfg->bill.ss7.userId) {
        disp = TCAPDispatcher::getInstance();
        _cfg->bill.ss7.userId += 39; //adjust USER_ID to PortSS7 units id
        if (!disp->connect(_cfg->bill.ss7.userId))
            smsc_log_error(logger, "%s: EINSS7 stack unavailable!!!", _logId);
        else {
            smsc_log_debug(logger, "%s: TCAP dispatcher has connected to SS7 stack", _logId);
            if (!disp->openSSN(_cfg->bill.ss7.own_ssn, _cfg->bill.ss7.maxDlgId))
                smsc_log_error(logger, "%s: SSN[%u] unavailable!!!", _logId, _cfg->bill.ss7.own_ssn);
        }
    }                                         

    _cfg->bill.abCache = (abCache = new AbonentCacheMTR(&_cfg->cachePrm, logger));
    assert(_cfg->bill.abCache);
    smsc_log_debug(logger, "%s: AbonentCache inited", _logId);

    //initialize IAProviders for defined policies
    for (AbonentPolicies::iterator pit = _cfg->abPolicies.begin(); 
                                    pit != _cfg->abPolicies.end(); pit++) {
        AbonentPolicy *pol = *pit;
        pol->getIAProvider();
    }

    INPSerializer::getInstance();
    server = new Server(&_cfg->sock, logger);
    assert(server);
    server->addListener(this);
    smsc_log_debug(logger, "%s: TCP server inited", _logId);

    if (_cfg->bill.cdrMode) {
        _cfg->bill.bfs = new InBillingFileStorage(_cfg->bill.cdrDir, 0, logger);
        assert(_cfg->bill.bfs);
        int oldfs = _cfg->bill.bfs->RFSOpen(true);
        assert(oldfs >= 0);
        smsc_log_debug(logger, "%s: Billing storage opened%s", _logId,
                       oldfs > 0 ? ", old files rolled": "");

        if (_cfg->bill.cdrInterval) { //use external storage roller
            roller = new InFileStorageRoller(_cfg->bill.bfs,
                                             (unsigned long)_cfg->bill.cdrInterval);
            assert(roller);
            smsc_log_debug(logger, "%s: BillingStorage roller inited", _logId);
        }
    } else
        _cfg->bill.bfs = NULL;

    _cfg->bill.twReg = new TimeWatchersRegistry(logger);
    assert(_cfg->bill.twReg);
    _cfg->bill.schedMgr = this;
    smsc_log_debug(logger, "%s: TimeWatcher inited", _logId);
}

Service::~Service()
{
    smsc_log_debug(logger, "%s: Releasing ..", _logId);
    if (running)
      stop();

    if (disp) {
        smsc_log_debug(logger, "%s: Disconnecting SS7 stack ..", _logId);
        disp->disconnect();
    }

    if (server) {
        server->removeListener(this);
        smsc_log_debug( logger, "%s: Deleting TCP server ..", _logId);
        delete server;
    }
    if (_cfg->bill.bfs) {
        smsc_log_debug(logger, "%s: Closing Billing storage ..", _logId);
        _cfg->bill.bfs->RFSClose();
        if (roller)
            delete roller;
        delete _cfg->bill.bfs;
    }
    if (_cfg->bill.twReg) {
        smsc_log_debug(logger, "%s: Deleting TimeWatchers ..", _logId);
        delete _cfg->bill.twReg;
    }
    if (abCache) {
        smsc_log_debug(logger, "%s: Closing AbonentsCache ..", _logId);
        delete abCache;
    }
    //delete task schedulers if any
    for (TSchedList::iterator it = tschedList.begin(); it != tschedList.end(); ++it) {
        smsc_log_debug(logger, "%s: deleting %s ..", _logId, it->second->Name());
        delete it->second;
    }

    delete _cfg;
    smsc_log_debug(logger, "%s: Released.", _logId);
}

bool Service::start()
{
    smsc_log_debug(logger, "%s: Starting TCP server ..", _logId);
    if (!server->Start())
        return false;
    
    if (roller) {
        smsc_log_debug(logger, "%s: Starting BillingStorage roller ..", _logId);
        roller->Start();
    }
    running = true;
    smsc_log_debug(logger, "%s: Started.", _logId);
    return running;
}

void Service::stop()
{
    if (server) {
        smsc_log_debug(logger, "%s: Stopping TCP server ..", _logId);
        server->Stop();
    }

    smsc_log_debug(logger, "%s: Stopping TimeWatchers ..", _logId);
    _cfg->bill.twReg->StopAll();
    
    if (disp) {
        smsc_log_debug(logger, "%s: Stopping TCAP dispatcher ..", _logId);
        disp->Stop();
    }

    if (roller) {
        smsc_log_debug(logger, "%s: Stopping BillingStorage roller ..", _logId);
        roller->Stop(true);
    }

    //stop task schedulers if any
    for (TSchedList::iterator it = tschedList.begin(); it != tschedList.end(); ++it) {
        smsc_log_debug(logger, "%s: stoping %s ..", _logId, it->second->Name());
        it->second->Stop();
    }

    running = false;
    smsc_log_debug(logger, "%s: Stopped.", _logId);
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
            smsc_log_info(logger, "%s: closing %s session[%u] on Connect[%u]", _logId,
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
//    smsc_log_debug(logger, "%s: New Connect[%u] inited", _logId, conn->getId());
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
    smsc_log_debug(logger, "%s: TCP server shutdowning, reason %d", _logId, reason);
    srv->removeListener(this);

    if (reason != Server::srvStopped) { //abnormal shutdown
        throw CustomException("%s: TCP server fatal failure, exiting.");
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
        MutexGuard tmp(_mutex);
        newSess.sId = ++lastSessId;
        newSess.hdl = new BillingManager(&_cfg->bill, newSess.sId, conn, logger);
    } break;

    case smsc::inman::interaction::csAbntContract: {
        MutexGuard tmp(_mutex);
        newSess.sId = ++lastSessId;

        AbonentDetectorCFG sCfg;
        sCfg.twReg = _cfg->bill.twReg;
        sCfg.abCache = _cfg->bill.abCache;
        sCfg.policies = _cfg->bill.policies;
        sCfg.abtTimeout = _cfg->bill.abtTimeout;
        sCfg.maxRequests = _cfg->bill.maxBilling;
        sCfg.ss7 = _cfg->bill.ss7;

        newSess.hdl = new AbntDetectorManager(&sCfg, newSess.sId, conn, logger);
    } break;

    default: //force connect closing by TCPSrv
        throw CustomException("%s: unsupported CommandSet: %s (%u)",
                              pCs->CsName(), pCs->CsId());
    }
    _mutex.Lock();
    sockets.insert(SocketsMap::value_type(conn->getId(), newSess.sId));
    sessions.insert(SessionsMap::value_type(newSess.sId, newSess));
    _mutex.Unlock();
    smsc_log_info(logger, "%s: New %s session[%u] on Connect[%u] created", _logId,
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


TaskSchedulerITF * Service::getScheduler(TaskSchedulerITF::SchedulerType sched_type)
{
    {
        MutexGuard grd(_mutex);
        for (TSchedList::iterator it = tschedList.begin(); it != tschedList.end(); ++it) {
            if (it->first == sched_type)
                return it->second;
        }
    }
    //create new scheduler
    std::auto_ptr<TaskSchedulerAC> pShed;
    if (sched_type == TaskSchedulerITF::schedMT)
        pShed.reset(new TaskSchedulerMT(logger));
    else
        pShed.reset(new TaskSchedulerSEQ(logger));

    if (pShed->Start()) {
        MutexGuard grd(_mutex);
        tschedList.push_back(TSchedRCD(sched_type, pShed.get()));
        return pShed.release();
    }
    smsc_log_error(logger, "%s: failed to start %s ..", _logId, pShed->Name());
    return NULL;
}

} // namespace inmgr
} // namespace smsc
