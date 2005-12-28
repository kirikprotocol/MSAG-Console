static char const ident[] = "$Id$";
#include <assert.h>

#include "inman/common/synch.hpp"
#include "inman/interaction/server.hpp"
#include "inman/common/util.hpp"
#include "inman/common/errors.hpp"

using smsc::inman::common::Synch;
using smsc::inman::common::format;
using smsc::inman::common::SystemError;

namespace smsc  {
namespace inman {
namespace interaction  {

//timeout for select()
#define TIMEOUT_STEP 100 //millisecs

Server::Server(const char* host, int port, SerializerITF * serializer,
               unsigned int timeout_secs/* = 20*/, unsigned short max_conn/* = 10*/,
               Logger* uselog/* = NULL*/)
        : ipSerializer(serializer), _maxConn(max_conn)
        , _runState(Server::lstStopping), logger(uselog)
{
    assert(host && serializer);
    if (!logger)
        logger = Logger::getInstance("smsc.inman.TCPSrv");

    if (serverSocket.InitServer(host, port, timeout_secs))
        throw SystemError(format("TCPSrv: failed to init server socket %s:%d", host, port));

    if(serverSocket.StartServer())
        throw SystemError(format("TCPSrv: failed to start server socket %s:%d", host, port));
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
        ConnectsList    cplist = connects;
        smsc_log_debug(logger, "TCPSrv: %s %u connects ..", abort ? "killing" : "closing",
                       cplist.size());
        for (ConnectsList::const_iterator cit = cplist.begin(); cit != cplist.end(); cit++)
            closeConnect(*cit, abort);
    }
}

void Server::openConnect(Connect* connect)
{
    assert(connect);
    _mutex.Lock();
    connects.push_back( connect );
    _mutex.Unlock();

    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
        ServerListener* ptr = *it;
        ptr->onConnectOpened(this, connect);
    }
}

void Server::closeConnect(Connect* connect, bool abort/* = false*/)
{
    assert(connect);
    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
        ServerListener* ptr = *it;
        ptr->onConnectClosing(this, connect);
    }
    _mutex.Lock();
    connects.remove(connect);
    _mutex.Unlock();

    SOCKET sockId = connect->getSocketId();
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

    for (timeOutMilliSecs += TIMEOUT_STEP;
          timeOutMilliSecs > 0; timeOutMilliSecs -= TIMEOUT_STEP) {
        if (_runState == Server::lstStopped)
            break;
        usleep(1000*TIMEOUT_STEP); //microsecs
    }

    _mutex.Lock();
    if (_runState != Server::lstStopped) {
        smsc_log_debug(logger, "TCPSrv: timeout expired, %u connects are still active",
                       connects.size());
        _runState = Server::lstStopped;  //forcedly stop Listener thread
    }
    _mutex.Unlock();
}


Server::ShutdownReason Server::Listen()
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
                smsc_log_debug(logger, "TCPSrv: Event on socket[%u]", socket);
                if (!conn->process()) {
                    CustomException * exc = conn->hasException();
                    if (exc) {
                        smsc_log_error(logger, "TCPSrv: %s", exc->what());
                        conn->resetException();
                    } else //remote point ends connection
                        closeConnect(conn);
                }
            }
            if (FD_ISSET(socket, &errorSet)) {
                smsc_log_debug(logger, "TCPSrv: Error Event on socket[%u].", socket);
                conn->handleConnectError(true);
                closeConnect(conn, true);
            }
        }

        if (_runState == Server::lstRunning) {
            if (FD_ISSET(serverSocket.getSocket(), &readSet)) {
                Socket* clientSocket = serverSocket.Accept();
                if (!clientSocket) {
                    smsc_log_error(logger, "TCPSrv: failed to accept client connection, cause: %d (%s)",
                                   errno, strerror(errno));
                } else if (connects.size() < _maxConn) {
                    smsc_log_debug(logger, "TCPSrv: accepted new connect[%u]",
                                   clientSocket->getSocket());
                    Connect* connect = new Connect(clientSocket, ipSerializer,
                                                   Connect::frmStraightData, logger);
                    openConnect(connect);
                    if (!connect->hasListeners()) {
                        smsc_log_warn(logger, "TCPSrv: No listeners being set for connect[%u]!",
                                      connect->getSocketId());
                        closeConnect(connect);
                    }
                } else { //connects number exceeded
                    delete clientSocket;
                    smsc_log_warn(logger, "TCPSrv: connection refused, resource limitation.");
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
    return result;
}


// Thread entry point
int Server::Execute()
{
    Server::ShutdownReason result = Server::srvStopped;
    try {
        smsc_log_debug(logger, "TCPSrv: Listener thread started.");
        result = Listen();
    } catch (const std::exception& error) {
        smsc_log_error(logger, "TCPSrv: Listener thread failure: %s", error.what());
        result = Server::srvUnexpected;
    }
    smsc_log_debug(logger, "TCPSrv: Listener thread finished, cause %d", (int)result);

    //forcedly close active connects
    closeAllConnects(result == Server::srvStopped ? false : true);
    
    //notify ServerListeners about server shutdown
    ListenerList cplist = listeners;
    for (ListenerList::iterator it = cplist.begin(); it != cplist.end(); it++) {
        ServerListener* ptr = *it;
        ptr->onServerShutdown(this, result);
    }
    return result;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

