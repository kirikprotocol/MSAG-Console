#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/interaction/ConnSrv.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

#define LISTENER_RESTART_ATTEMPTS   2
/* ************************************************************************** *
 * class ConnectSrv implementation:
 * ************************************************************************** */
ConnectSrv::ConnectSrv(unsigned tmo_msecs/* = POLL_TIMEOUT_ms*/, Logger * uselog /*= NULL*/)
    : tmoMSecs(tmo_msecs), lstRestartCnt(0), _runState(ConnectSrv::lstStopped), logger(uselog)
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
    MutexGuard grd(_mutex);
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
    }
    return socket.release();
}

unsigned ConnectSrv::addConnection(ConnectAC * use_conn, ConnectSupervisorITF * mgr)
{
    ConnectInfo connInfo(use_conn, mgr);
    MutexGuard grd(_mutex);
    connects.insert(ConnectsMap::value_type(use_conn->getId(), connInfo));
    return use_conn->getId();
}

ConnectAC * ConnectSrv::rlseConnection(unsigned conn_id)
{
    ConnectAC * conn = NULL;
    MutexGuard grd(_mutex);
    ConnectsMap::iterator it = connects.find(conn_id);
    if (it != connects.end()) {
        conn = ((*it).second).conn;
        connects.erase(it);
        smsc_log_debug(logger, "ConnSrv: Connect[%u] released.", conn->getId());
    }
    return conn;
}

bool ConnectSrv::Start(void)
{
    Thread::Start();
    if (lstEvent.Wait(POLL_TIMEOUT_ms)) {
        smsc_log_error(logger, "ConnSrv: unable to start Listener thread!");
        return false;
    }
    return true;
}

void ConnectSrv::Stop(unsigned int timeOut_msecs/* = 400*/)
{
    {
        MutexGuard grd(_mutex);
        if (_runState == ConnectSrv::lstStopped)
            return;

        //adjust timeout for POLL_TIMEOUT_ms millsecs
        timeOut_msecs = POLL_TIMEOUT_ms*((timeOut_msecs + (POLL_TIMEOUT_ms/2))/POLL_TIMEOUT_ms);
        smsc_log_debug(logger, "ConnSrv: stopping Listener thread, timeout = %u ms ..",
                       timeOut_msecs);
        if (_runState == ConnectSrv::lstRunning)
            _runState = ConnectSrv::lstStopping;
    }
    lstEvent.Wait(timeOut_msecs);
    {
        MutexGuard grd(_mutex);
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
    _mutex.Lock();
    if (!connects.empty()) {
        smsc_log_debug(logger, "ConnSrv: %s %u connects ..", abort ? "killing" : "closing",
                       connects.size());
        do {
            ConnectsMap::iterator it = connects.begin();
            ConnectAC * conn = ((*it).second).conn;
            unsigned conn_id = conn->getId();
            ConnectSupervisorITF * mgr = ((*it).second).mgr;
            connects.erase(it);
            conn->Close(abort);
            smsc_log_debug(logger, "ConnSrv: Connect[%u] %s.", conn_id,
                           abort ? "aborted" : "closed");
            _mutex.Unlock();
            if (mgr && !mgr->onConnectClosed(conn))
                conn = NULL;
            if (conn) {
                smsc_log_debug(logger, "ConnSrv: Connect[%u] deleted", conn_id);
                delete conn;
            }
            _mutex.Lock();
        } while (!connects.empty());
    }
    _mutex.Unlock();
}

void ConnectSrv::closeConnect(unsigned conn_id, bool abort/* = false*/)
{
    ConnectAC * conn = NULL;
    ConnectSupervisorITF * mgr = NULL;
    {
        MutexGuard grd(_mutex);
        ConnectsMap::iterator it = connects.find(conn_id);
        if (it == connects.end())
            return;
        
        conn = ((*it).second).conn;
        mgr = ((*it).second).mgr;
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
    _mutex.Lock();
    _runState = ConnectSrv::lstRunning;
    while (_runState != ConnectSrv::lstStopped) {
        //check for last connect while stopping
        if ((_runState == ConnectSrv::lstStopping) && !connects.size()) {
            _runState = ConnectSrv::lstStopped;
            _mutex.Unlock();
            smsc_log_debug(logger, "ConnSrv: all connects finished, stopping ..");
            break;
        }

        int     n, maxSock = 0;
        fd_set  readSet;
        fd_set  errorSet;

        FD_ZERO(&readSet);
        FD_ZERO(&errorSet);

        //in case of lstStopping the clients connection should be served for a while
        for (ConnectsMap::iterator i = connects.begin(); i != connects.end(); i++) {
            SOCKET  socket = ((*i).second).conn->getId();
            FD_SET(socket, &readSet);
            FD_SET(socket, &errorSet);
            if (socket > maxSock)
                maxSock = socket;
        }
        _mutex.Unlock();

        if (!(n = select(maxSock+1, &readSet, 0, &errorSet, &tmo))) { //timeout expired
            _mutex.Lock(); continue;
        }
            
        if (n < 0) {
            smsc_log_fatal(logger, "ConnSrv: select() failed, cause: %d (%s)",
                           errno, strerror(errno));
            result = ConnectSrv::srvError;
            _mutex.Lock();
            _runState = ConnectSrv::lstStopped;
            _mutex.Unlock();
        }
        if (_runState == ConnectSrv::lstStopped)
            break;

        //iterate over connect set copy in order to safely modify original collection
        _mutex.Lock();
        ConnectsMap stump(connects);
        _mutex.Unlock();
        for (ConnectsMap::iterator i = stump.begin(); i != stump.end(); i++) {
            ConnectAC* conn = ((*i).second).conn;
            unsigned socket = conn->getId();

            if (FD_ISSET((SOCKET)socket, &readSet)) {
                ConnectAC::ConnectState  st = ConnectAC::connAlive;
                try { st = conn->onReadEvent(); }
                catch (const std::exception & exc) {
                    smsc_log_error(logger, "ConnSrv: Connect[%u] exception: %s", socket, exc.what());
                    st = ConnectAC::connException;
                }
                if (st == ConnectAC::connEOF)
                    smsc_log_debug(logger, "ConnSrv: remote point ends Connect[%u]", socket);
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
        _mutex.Lock();
    } /* eow */
    _mutex.Unlock();
    lstEvent.Signal();
    return result;
}

// Thread entry point
int ConnectSrv::Execute()
{
    lstEvent.Signal();
    ConnectSrv::ShutdownReason result = ConnectSrv::srvStopped;
    do {
        try {
            smsc_log_debug(logger, "ConnSrv: Listener started.");
            result = Listen();
        } catch (const std::exception& error) {
            smsc_log_error(logger, "ConnSrv: Listener unexpected failure: %s", error.what());
            result = ConnectSrv::srvUnexpected;
            lstRestartCnt++;
        }
        smsc_log_debug(logger, "ConnSrv: Listener finished, cause %d", (int)result);
        //forcedly close active connects
        closeAllConnects(true);
    } while ((result == ConnectSrv::srvUnexpected)
             && (lstRestartCnt < LISTENER_RESTART_ATTEMPTS));
    return result;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

