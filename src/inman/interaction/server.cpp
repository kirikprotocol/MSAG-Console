static char const ident[] = "$Id$";
#include <assert.h>

#include "inman/interaction/server.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

//timeout for select()
#define TIMEOUT_STEP 100 //millisecs
#define LISTENER_RESTART_ATTEMPTS  2

Server::Server(const ServSocketCFG * in_cfg, Logger * uselog /*= NULL*/)
    : _cfg(*in_cfg), lstRestartCnt(0)
    , _runState(Server::lstStopped), logger(uselog)
{
    assert(_cfg.host.length());
    if (!logger)
        logger = Logger::getInstance("smsc.inman.TCPSrv");
}

Server::~Server()
{
    _mutex.Lock();
    removeAllListeners();
    _mutex.Unlock();

    if (_runState != Server::lstStopped)
        Stop();
    WaitFor();

    if (serverSocket.getSocket() != INVALID_SOCKET) {
        serverSocket.Abort();
        smsc_log_debug(logger, "TCPSrv: server socket closed.");
    }
}

//Closes all client's connections
void Server::closeAllConnects(bool abort/* = false*/)
{
    if (connects.size()) {
        smsc_log_debug(logger, "TCPSrv: %s %u connects ..", abort ? "killing" : "closing",
                       connects.size());
        ConnectsList::iterator it = connects.begin();
        while (it != connects.end()) {
            ConnectsList::iterator cit = it; it++;
            closeConnect(*cit, abort);
            connects.erase(cit);
        } 
    }
}

void Server::openConnect(Socket* use_sock)
{
    unsigned sock_id = use_sock->getSocket();
    std::auto_ptr<Connect> pConn(new Connect(use_sock,
                                        Connect::frmStraightData, NULL, logger));
    try {
        for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
            ServerListener* ptr = *it;
            ptr->onConnectOpened(this, pConn.get());
        }
        if (!pConn->hasListeners())
            smsc_log_warn(logger, "TCPSrv: connect[%u] no listener set!", sock_id);
        else
            connects.push_back(pConn.release());
    } catch (const std::exception& exc) {
        smsc_log_error(logger, "TCPSrv: connect[%u] opening listener exception: %s",
                       sock_id, exc.what());
    }
    if (pConn.get()) {
        pConn->close(true);
        smsc_log_debug(logger, "TCPSrv: Socket[%u] closed.", sock_id);
    }
}

void Server::closeConnect(Connect* connect, bool abort/* = false*/)
{
    SOCKET sockId = connect->getSocketId();
    try {
        for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
            ServerListener* ptr = *it;
            ptr->onConnectClosing(this, connect);
        }
    } catch (const std::exception& exc) {
        smsc_log_error(logger, "TCPSrv: connect[%u] closing listener exception: %s",
                       (unsigned)sockId, exc.what());
    }
    connect->close(abort);
    delete connect;
    smsc_log_debug(logger, "TCPSrv: Socket[%u] closed.", (unsigned)sockId);
}

void Server::Stop(unsigned int timeOutMilliSecs/* = 400*/)
{
    if (_runState == Server::lstStopped)
        return;

    //adjust timeout for TIMEOUT_STEP millsecs
    timeOutMilliSecs = TIMEOUT_STEP*((timeOutMilliSecs + (TIMEOUT_STEP/2))/TIMEOUT_STEP);

    smsc_log_debug(logger, "TCPSrv: stopping Listener thread, timeout = %u ms ..",
                   timeOutMilliSecs);
    _mutex.Lock();
    if (_runState == Server::lstRunning)
        _runState = Server::lstStopping;
    _mutex.Unlock();

    lstEvent.Wait(timeOutMilliSecs);

    _mutex.Lock();
    if (_runState != Server::lstStopped) {
        smsc_log_debug(logger, "TCPSrv: timeout expired, %u connects are still active",
                       connects.size());
        _runState = Server::lstStopped;  //forcedly stop Listener thread
    }
    _mutex.Unlock();
    WaitFor();
}


Server::ShutdownReason Server::Listen(void)
{
    struct timeval tmo;
    Server::ShutdownReason result = Server::srvStopped;

    tmo.tv_sec = 0;
    tmo.tv_usec = 1000L*TIMEOUT_STEP;

    _mutex.Lock();
    _runState = Server::lstRunning;
    while (_runState != Server::lstStopped) {
        //check for last connect while stopping
        if ((_runState == Server::lstStopping) && !connects.size()) {
            _runState = Server::lstStopped;
            _mutex.Unlock();
            smsc_log_debug(logger, "TCPSrv: all connects finished, stopping ..");
            break;
        }

        int     n, maxSock = 0;
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
            SOCKET  socket = (*i)->getSocketId();
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
            smsc_log_fatal(logger, "TCPSrv: select() failed, cause: %d (%s)",
                           errno, strerror(errno));
            result = Server::srvError;
            _mutex.Lock();
            _runState = Server::lstStopped;
            _mutex.Unlock();
        }
        if (_runState == Server::lstStopped)
            break;

        //Serve clients connects first (iterate over list copy in order
        //to safely modify original collection)
        _mutex.Lock();
        ConnectsList stump(connects);
        _mutex.Unlock();
        for (ConnectsList::iterator i = stump.begin(); i != stump.end(); i++) {
            Connect* conn = (*i);
            SOCKET socket = conn->getSocketId();

            if (FD_ISSET(socket, &readSet)) {
                if (!conn->process()) {
                    CustomException * exc = conn->hasException();
                    if (exc) {
                        smsc_log_error(logger, "TCPSrv: %s", exc->what());
                        conn->resetException();
                    } else { //remote point ends connection
                        smsc_log_debug(logger, "TCPSrv: client ends connection[%u]", socket);
                        connects.remove(conn);
                        closeConnect(conn);
                    }
                }
            }
            if (FD_ISSET(socket, &errorSet)) {
                smsc_log_debug(logger, "TCPSrv: Error Event on socket[%u].", socket);
                conn->handleConnectError(true);
                connects.remove(conn);
                closeConnect(conn, true);
            }
        }

        if (_runState == Server::lstRunning) {
            if (FD_ISSET(serverSocket.getSocket(), &readSet)) {
                Socket* clientSocket = serverSocket.Accept();
                if (!clientSocket) {
                    smsc_log_error(logger, "TCPSrv: failed to accept client connection, cause: %d (%s)",
                                   errno, strerror(errno));
                } else if (connects.size() < _cfg.maxConn) {
                    smsc_log_debug(logger, "TCPSrv: accepted new connect[%u]",
                                   clientSocket->getSocket());
                    openConnect(clientSocket);
                } else { //connects number exceeded
                    delete clientSocket;
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
        _mutex.Lock();
    } /* eow */
    _mutex.Unlock();
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
    ListenerList cplist = listeners;
    for (ListenerList::iterator it = cplist.begin(); it != cplist.end(); it++) {
        ServerListener* ptr = *it;
        ptr->onServerShutdown(this, result);
    }
    return result;
}

bool Server::Start(void)
{
    if (serverSocket.InitServer(_cfg.host.c_str(), _cfg.port, _cfg.timeout)) {
        smsc_log_fatal(logger, "TCPSrv: failed to init server socket %s:%d",
                       _cfg.host.c_str(), _cfg.port);
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

} // namespace interaction
} // namespace inman
} // namespace smsc

