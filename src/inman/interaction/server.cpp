#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <assert.h>

#include "inman/interaction/server.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

#define TIMEOUT_STEP                100 //default timeout for select(), millisecs
#define LISTENER_RESTART_ATTEMPTS   2
/* ************************************************************************** *
 * class Service implementation:
 * ************************************************************************** */
Server::Server(const ServSocketCFG * in_cfg, Logger * uselog /*= NULL*/)
    : _cfg(*in_cfg), _runState(Server::lstStopped), lstRestartCnt(0), logger(uselog)
{
    assert(_cfg.host.length());
    if (!logger)
        logger = Logger::getInstance("smsc.inman.TCPSrv");
    setPollTimeout(TIMEOUT_STEP);
}

Server::~Server()
{
    clearListeners();
    if (_runState != Server::lstStopped)
        Stop();
    WaitFor();

    if (serverSocket.getSocket() != INVALID_SOCKET) {
        serverSocket.Abort();
        smsc_log_debug(logger, "TCPSrv: server socket closed.");
    }
}

void Server::setPollTimeout(unsigned long milli_secs)
{
    if (!milli_secs)
        milli_secs = TIMEOUT_STEP;

    tmo.tv_sec = milli_secs/1000;
    tmo.tv_usec = 1000L*(milli_secs % 1000);
}

//Ad hoc method: upon successfull socket initialization,
//calls listeners via onConnectOpening()
bool Server::setConnection(const char * host, unsigned port, unsigned timeout_secs)
{
    std::auto_ptr<Socket> socket(new Socket());
    if (socket->Init(host, port, timeout_secs) || socket->Connect()) {
        smsc_log_error(logger, "TCPSrv: unable to set connection to %s:%u : %s (%d)",
                       host, port, strerror(errno), errno);
        return false;
    }
    openConnect(socket);
    return true;
}

bool Server::Start(void)
{
    if (serverSocket.InitServer(_cfg.host.c_str(), _cfg.port, _cfg.timeout, 1, true)) {
        smsc_log_fatal(logger, "TCPSrv: failed to init server socket %s:%d : %d (%s)",
                       _cfg.host.c_str(), _cfg.port, errno, strerror(errno));
        return false;
    }
    if (serverSocket.StartServer()) {
        smsc_log_fatal(logger, "TCPSrv: failed to start server socket %s:%d",
                       _cfg.host.c_str(), _cfg.port);
        return false;
    }

    Thread::Start();
    if (lstEvent.Wait(TIMEOUT_STEP)) {
        smsc_log_error(logger, "TCPSrv: unable to start Listener thread");
        return false;
    }
    return true;
}

void Server::Stop(unsigned int timeOutMilliSecs/* = _SHUTDOWN_TMO_MS*/)
{
    {
        MutexGuard  tmp(Sync());
        if (_runState == Server::lstStopped)
            return;
        _runState = Server::lstStopping;
        if (!timeOutMilliSecs) {
            smsc_log_debug(logger, "TCPSrv: stopping Listener ..");
            return;
        }
        //adjust timeout for TIMEOUT_STEP millsecs
        timeOutMilliSecs = TIMEOUT_STEP*((timeOutMilliSecs + (TIMEOUT_STEP/2))/TIMEOUT_STEP);
        smsc_log_debug(logger, "TCPSrv: stopping Listener, timeout = %u ms ..",
                       timeOutMilliSecs);
    }
    lstEvent.Wait(timeOutMilliSecs);
    {
        MutexGuard  tmp(Sync());
        if (_runState != Server::lstStopped) {
            _runState = Server::lstStopped;  //forcedly stop Listener thread
            smsc_log_debug(logger, "TCPSrv: timeout expired, %u connects are still active",
                           connects.size());
        }
    }
    Thread::WaitFor();
}

/* -------------------------------------------------------------------------- *
 * Private/Protected methods:
 * -------------------------------------------------------------------------- */
//Closes all client's connections
void Server::closeAllConnects(bool abort/* = false*/)
{
    if (!connects.empty()) {
        smsc_log_debug(logger, "TCPSrv: %s %u connects ..", abort ? "killing" : "closing",
                       connects.size());
        ConnectsList::iterator it = connects.begin();
        while (it != connects.end()) {
            ConnectsList::iterator cit = it++;
            closeConnect(*cit, abort);
            connects.erase(cit);
        } 
    }
}
//Notifies listeners for new socket opened. In case of ConnectAC is created
//by listeners, add it to connects collection.
//NOTE: Requires Sync() to be unlocked !!!
void Server::openConnect(std::auto_ptr<Socket>& use_sock)
{
    use_sock->SetNoDelay(true);
    unsigned sock_id = use_sock->getSocket();
    std::auto_ptr<ConnectAC> pConn;
    GRDNode *it = begin();
    try {
        for (; it && !pConn.get(); it = next(it)) {
            pConn.reset(it->val->onConnectOpening(this, use_sock.get()));
        }
    } catch (const std::exception& exc) {
        smsc_log_error(logger, "TCPSrv: connect[%u] opening listener exception: %s",
                       sock_id, exc.what());
    }
    if (it)
        it->unmark();

    if (pConn.get() && (pConn->State() == ConnectAC::connAlive)) {
        MutexGuard grd(Sync());
        use_sock.release();
        connects.push_back(pConn.release());
    } else {
        use_sock->Abort();
        smsc_log_warn(logger, "TCPSrv: Socket[%u] closed: no listener set!", sock_id);
    }
}

//NOTE: Requires Sync() to be unlocked !!!
void Server::closeConnect(ConnectAC* connect, bool abort/* = false*/)
{
    unsigned sockId = connect->getId();
    try {
        for (GRDNode *it = begin(); it; it = next(it))
            it->val->onConnectClosing(this, connect);
    } catch (const std::exception& exc) {
        smsc_log_error(logger, "TCPSrv: connect[%u] closing listener exception: %s",
                       (unsigned)sockId, exc.what());
    }
    connect->Close(abort);
    delete connect;
    smsc_log_info(logger, "TCPSrv: Socket[%u] closed.", (unsigned)sockId);
}


void Server::closeConnectGuarded(ConnectsList::iterator & it, bool abort/* = false*/)
{
    ConnectAC* connect = *it;
    {
        MutexGuard grd(Sync());
        connects.erase(it);
    }
    closeConnect(connect, abort);
}

Server::ShutdownReason Server::Listen(void)
{
    Server::ShutdownReason result = Server::srvStopped;
    Sync().Lock();
    _runState = Server::lstRunning;
    while (_runState != Server::lstStopped) {
        //check for last connect while stopping
        if ((_runState == Server::lstStopping) && connects.empty()) {
            _runState = Server::lstStopped;
            smsc_log_debug(logger, "TCPSrv: all connects finished, stopping ..");
            break;
        }

        int     maxSock = 0;
        fd_set  readSet;
        fd_set  errorSet;

        FD_ZERO(&readSet);
        FD_ZERO(&errorSet);

        if (_runState == Server::lstRunning) {
            FD_SET(serverSocket.getSocket(), &readSet);
            FD_SET(serverSocket.getSocket(), &errorSet);
            maxSock = serverSocket.getSocket();
        } //in case of lstStopping the clients connection should be served for a while
        for (ConnectsList::iterator i = connects.begin(); i != connects.end(); i++) {
            SOCKET  socket = (*i)->getId();
            FD_SET(socket, &readSet);
            FD_SET(socket, &errorSet);
            if (socket > maxSock)
                maxSock = socket;
        }
        Sync().Unlock();

        int n = select(maxSock+1, &readSet, 0, &errorSet, &tmo);
        Sync().Lock();
        if (!n) //timeout expired
            continue;
        if (n < 0) {
            smsc_log_fatal(logger, "TCPSrv: select() failed, cause: %d (%s)",
                           errno, strerror(errno));
            result = Server::srvError;
            _runState = Server::lstStopped;
        }
        if (_runState == Server::lstStopped)
            break;
        Sync().Unlock();

         //Serve clients connects first
        ConnectsList::iterator nit = connects.begin();
        while (nit != connects.end()) {
            ConnectsList::iterator it = nit++;
            ConnectAC* conn = (*it);
            SOCKET socket = (SOCKET)conn->getId();

            if (FD_ISSET(socket, &readSet)) {
                ConnectAC::ConnectState  st = conn->onReadEvent();
                if (st == ConnectAC::connEOF) {
                    smsc_log_debug(logger, "TCPSrv: client ends Connect[%u]", socket);
                } else if (st != ConnectAC::connAlive)
                    closeConnectGuarded(it);
            }
            if (FD_ISSET(socket, &errorSet)) {
                smsc_log_debug(logger, "TCPSrv: Error Event on socket[%u].", socket);
                conn->onErrorEvent();
                closeConnectGuarded(it, true);
            }
        }

        if (_runState == Server::lstRunning) {
            if (FD_ISSET(serverSocket.getSocket(), &readSet)) {
                std::auto_ptr<Socket> clientSocket(serverSocket.Accept());
                if (!clientSocket.get()) {
                    smsc_log_error(logger, "TCPSrv: failed to accept client connection, cause: %d (%s)",
                                   errno, strerror(errno));
                } else if (connects.size() < _cfg.maxConn) {
                    smsc_log_debug(logger, "TCPSrv: accepted new connect[%u]",
                                   clientSocket->getSocket());
                    openConnect(clientSocket);
                } else { //connects number exceeded
                    smsc_log_warn(logger, "TCPSrv: connection refused, "
                                          "resource limitation: %u", _cfg.maxConn);
                }
            }
            if (FD_ISSET(serverSocket.getSocket(), &errorSet)) {
                smsc_log_fatal(logger, "TCPSrv: Error on server socket, stopping ..");
                result = Server::srvError;
                _runState = Server::lstStopping;
                serverSocket.Abort();
                smsc_log_debug(logger, "TCPSrv: server socket closed");
            }
        }
        Sync().Lock();
    } /* eow */
    Sync().Unlock();
    lstEvent.Signal();
    return result;
}

// Thread entry point
int Server::Execute()
{
    lstEvent.Signal();
    Server::ShutdownReason result = Server::srvStopped;
    do {
        try {
            smsc_log_debug(logger, "TCPSrv: Listener started.");
            result = Listen();
        } catch (const std::exception& error) {
            smsc_log_error(logger, "TCPSrv: Listener unexpected failure: %s", error.what());
            result = Server::srvUnexpected;
            lstRestartCnt++;
        }
        smsc_log_debug(logger, "TCPSrv: Listener finished, cause %d", (int)result);
        //forcedly close active connects
        closeAllConnects(true);
    } while ((result == Server::srvUnexpected)
             && (lstRestartCnt < LISTENER_RESTART_ATTEMPTS));
    
    //notify ServerListeners about server shutdown
    for (GRDNode *it = begin(); it; it = next(it))
        it->val->onServerShutdown(this, result);
    return result;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

