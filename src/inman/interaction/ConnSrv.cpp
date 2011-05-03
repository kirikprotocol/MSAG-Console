#ifndef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/ConnSrv.hpp"
using smsc::core::synchronization::MutexGuard;

namespace smsc  {
namespace inman {
namespace interaction  {

#define LISTENER_RESTART_ATTEMPTS   2
/* ************************************************************************** *
 * class ConnectSrv implementation:
 * ************************************************************************** */
ConnectSrv::ConnectSrv(unsigned tmo_msecs/* = POLL_TIMEOUT_ms*/, Logger * uselog /*= NULL*/)
    : tmoMSecs(tmo_msecs), _runState(ConnectSrv::lstStopped)
    , lstRestartCnt(0), logger(uselog)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.TCPSrv");
    tmo.tv_sec = tmoMSecs/1000;
    tmo.tv_usec = 1000L*(tmoMSecs % 1000);
}

ConnectSrv::~ConnectSrv()
{
    Stop();
    WaitFor();
}

ConnectSrv::SrvState ConnectSrv::State(void)
{
    MutexGuard grd(_Sync);
    return _runState;
}
//
Socket * ConnectSrv::setConnection(const char * host, unsigned port, unsigned timeout_secs)
{
    std::auto_ptr<Socket> socket(new Socket());
    if (socket->Init(host, port, timeout_secs) || socket->Connect()) {
        smsc_log_error(logger, "ConnSrv: unable to set connection to %s:%u : %s (%d)",
                       host, port, strerror(errno), errno);
        return NULL;
    } else {
      smsc_log_debug(logger, "ConnSrv: established connection[%u] to %s:%u",
                     (unsigned)socket->getSocket(), host, port);
    }
    return socket.release();
}

unsigned ConnectSrv::addConnection(ConnectAC * use_conn, ConnectSupervisorITF * mgr)
{
    ConnectInfo connInfo(use_conn, mgr);
    MutexGuard grd(_Sync);
    connects.insert(ConnectsMap::value_type(use_conn->getId(), connInfo));
    return use_conn->getId();
}

ConnectAC * ConnectSrv::rlseConnection(unsigned conn_id)
{
    ConnectAC * conn = NULL;
    _Sync.Lock();
    ConnectsMap::iterator it = connects.find(conn_id);
    if (it != connects.end()) {
        conn = (it->second).conn;
        (it->second).ignore = true;
        if (_runState != ConnectSrv::lstStopped)
            _Sync.wait(POLL_TIMEOUT_ms);
        connects.erase(it);
        smsc_log_debug(logger, "ConnSrv: Connect[%u] released.", conn->getId());
    }
    _Sync.Unlock();
    return conn;
}

unsigned ConnectSrv::numOfConnects(void)
{
    MutexGuard  tmp(_Sync);
    return (unsigned)connects.size();
}


bool ConnectSrv::Start(void)
{
    Thread::Start();
    int rc = Thread::getRetCode();
    if (rc) {
      smsc_log_error(logger, "ConnSrv: Listener thread start is failed: %s (%d)",
                     strerror(rc), rc);
      return false;
    }
    if ((rc = lstEvent.Wait(POLL_TIMEOUT_ms)) != 0) {
      smsc_log_error(logger, "ConnSrv: Listener thread start is timed out: %s (%d)",
                     strerror(rc), rc);
      return false;
    }
    return true;
}

void ConnectSrv::Stop(unsigned int timeOut_msecs/* = 400*/)
{
    {
        MutexGuard grd(_Sync);
        if (_runState == ConnectSrv::lstStopped)
            return;

        //adjust timeout for POLL_TIMEOUT_ms millsecs
        timeOut_msecs = POLL_TIMEOUT_ms*((timeOut_msecs + (POLL_TIMEOUT_ms/2))/POLL_TIMEOUT_ms);
        smsc_log_debug(logger, "ConnSrv: stopping Listener thread, timeout = %u ms ..",
                       timeOut_msecs);
        _runState = ConnectSrv::lstStopping;
    }
    lstEvent.Wait(timeOut_msecs);
    {
        MutexGuard grd(_Sync);
        if (_runState != ConnectSrv::lstStopped) {
            smsc_log_debug(logger, "ConnSrv: timeout expired, %u connects are still active",
                           connects.size());
            _runState = ConnectSrv::lstStopped;  //forcedly stop Listener thread
        }
    }
}

/* -------------------------------------------------------------------------- *
 * Private/Protected methods:
 * -------------------------------------------------------------------------- */
//Closes and deletes all connections
void ConnectSrv::closeAllConnects(bool abort/* = false*/)
{
    _Sync.Lock();
    if (!connects.empty()) {
        smsc_log_debug(logger, "ConnSrv: %s %u connects ..", abort ? "killing" : "closing",
                       connects.size());
        do {
            ConnectsMap::iterator it = connects.begin();
            ConnectAC * conn = ((*it).second).conn;
            unsigned conn_id = conn->getId();
            ConnectSupervisorITF * mgr = ((*it).second).mgr;
            connects.erase(it);
            _Sync.Unlock();

            conn->Close(abort);
            smsc_log_debug(logger, "ConnSrv: Connect[%u] %s.", conn_id,
                           abort ? "aborted" : "closed");
            if (mgr && !mgr->onConnectClosed(conn))
                conn = NULL;
            if (conn) {
                smsc_log_debug(logger, "ConnSrv: Connect[%u] deleted", conn_id);
                delete conn;
            }
            _Sync.Lock();
        } while (!connects.empty());
    }
    _Sync.Unlock();
}

void ConnectSrv::closeConnect(unsigned conn_id, bool abort/* = false*/)
{
    ConnectAC * conn = NULL;
    ConnectSupervisorITF * mgr = NULL;
    {
        ConnectsMap::iterator it = connects.find(conn_id);
        if (it == connects.end())
            return;
        
        conn = it->second.conn;
        mgr = it->second.mgr;
        connects.erase(it);
        conn->Close(abort);
        smsc_log_debug(logger, "ConnSrv: Connect[%u] %s.", conn_id,
                        abort ? "aborted" : "closed");
        
    }
    if (mgr && !mgr->onConnectClosed(conn))
        conn = NULL;
    if (conn) {
        smsc_log_debug(logger, "ConnSrv: Connect[%u] deleted", conn_id);
        delete conn;
    }
    return;
}

ConnectSrv::ShutdownReason ConnectSrv::Listen(void)
{
    ConnectSrv::ShutdownReason result = ConnectSrv::srvStopped;
    _Sync.Lock();
    _runState = ConnectSrv::lstRunning;
    while (_runState != ConnectSrv::lstStopped) {
        int     maxSock = 0;
        fd_set  readSet;
        fd_set  errorSet;

        FD_ZERO(&readSet);
        FD_ZERO(&errorSet);
        //in case of lstStopping the clients connection should be served for a while
        for (ConnectsMap::iterator it = connects.begin();
                            it != connects.end() && !it->second.ignore; ++it) {
            SOCKET  socket = it->second.conn->getId();
            FD_SET(socket, &readSet);
            FD_SET(socket, &errorSet);
            if (socket > maxSock)
                maxSock = socket;

        }
        if (!maxSock) { //check for last connect while stopping
            if ((_runState == ConnectSrv::lstStopping)) {
                _runState = ConnectSrv::lstStopped;
                smsc_log_debug(logger, "ConnSrv: all connects finished, stopping ..");
                break;
            }
            _Sync.notify();
            _Sync.wait(POLL_TIMEOUT_ms);
            continue;
        }
        _Sync.Unlock();
        int n = select(maxSock+1, &readSet, 0, &errorSet, &tmo);
        _Sync.Lock();
        _Sync.notify();
        if (!n) //timeout expired
            continue;
        if (n < 0) {
            smsc_log_fatal(logger, "ConnSrv: select() failed, cause: %d (%s)",
                           errno, strerror(errno));
            result = ConnectSrv::srvError;
            _runState = ConnectSrv::lstStopped;
            break;
        }
        for (ConnectsMap::iterator it = connects.begin();
                            it != connects.end() && !it->second.ignore; ++it) {
            ConnectAC* conn = it->second.conn;
            unsigned socket = conn->getId();

            if (FD_ISSET((SOCKET)socket, &readSet)) {
                ConnectAC::ConnectState  st = ConnectAC::connAlive;
                try { st = conn->onReadEvent(); }
                catch (const std::exception & exc) {
                    smsc_log_error(logger, "ConnSrv: Connect[%u] exception: %s", socket, exc.what());
                    st = ConnectAC::connException;
                }
                if (st == ConnectAC::connEOF) {
                    smsc_log_debug(logger, "ConnSrv: remote point ends Connect[%u]", socket);
                }
                if (st != ConnectAC::connAlive)
                    closeConnect(socket);
            }
            if (FD_ISSET((SOCKET)socket, &errorSet)) {
                smsc_log_debug(logger, "ConnSrv: Error Event on socket[%u].", socket);
                try { conn->onErrorEvent(); }
                catch (const std::exception & exc) {
                    smsc_log_error(logger, "ConnSrv: Connect[%u] exception: %s", socket, exc.what());
                }
                closeConnect(socket, true);
            }
        }
    } /* eow */
    _Sync.Unlock();
    return result;
}

// Thread entry point
int ConnectSrv::Execute()
{
    lstEvent.Signal();
    ConnectSrv::ShutdownReason result = ConnectSrv::srvStopped;
    do {
        try {
            smsc_log_debug(logger, "ConnSrv: Listener thread[%lu] started.",
                           (unsigned long)pthread_self());
            result = Listen();
        } catch (const std::exception& error) {
            smsc_log_error(logger, "ConnSrv: Listener unexpected failure: %s", error.what());
            result = ConnectSrv::srvUnexpected;
            lstRestartCnt++;
        }
        smsc_log_debug(logger, "ConnSrv: Listener finished, cause %d", (int)result);
    } while ((result == ConnectSrv::srvUnexpected)
             && (lstRestartCnt < LISTENER_RESTART_ATTEMPTS));
    //forcedly close active connects
    closeAllConnects(true);
    lstEvent.Signal();
    return result;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

