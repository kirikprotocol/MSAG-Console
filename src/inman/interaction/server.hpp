#ident "$Id$"
#ifndef __SMSC_INMAN_TCP_SERVER__
#define __SMSC_INMAN_TCP_SERVER__

#include <list>

#include "core/threads/Thread.hpp"
#include "core/synchronization/Mutex.hpp"
#include "inman/interaction/connect.hpp"

using smsc::core::threads::Thread;
using smsc::core::synchronization::Mutex;
using smsc::inman::interaction::Connect;

namespace smsc  {
namespace inman {
namespace interaction  {

class ServerListener;
class Server : public Thread, public ObservableT< ServerListener >
{
public:
    typedef std::list<Connect*> ConnectsList;
    typedef enum { lstStopped = 0, lstStopping, lstRunning } ServerState;
    typedef enum {
        srvUnexpected = -1, //listener caught unexpected fatal exception
        srvStopped = 0,     //normal shutdown
        srvError = 1        //server socket fatal error
    } ShutdownReason;

    Server(const char* szHost, int nPort, SerializerITF * serializer,
            unsigned int timeout_secs = 20, unsigned short max_conn = 10,
            Logger* uselog = NULL);
    virtual ~Server();

    void openConnect(Connect* connect);
    void closeConnect(Connect* connect, bool abort = false);

    int  Execute(); //listener thread entry point
    void Stop(unsigned int timeOutMilliSecs = 400);

protected:
    ShutdownReason Listen();
    //Closes all client's connections
    void closeAllConnects(bool abort = false);

    Mutex           _mutex;
    volatile
        ServerState _runState;
    unsigned short  _maxConn;

    SerializerITF * ipSerializer;
    Socket          serverSocket;
    ConnectsList    connects;
    Logger*         logger;
};

class ServerListener
{
public:
    virtual void onConnectOpened(Server* srv, Connect* conn) = 0;
    virtual void onConnectClosing(Server* srv, Connect* conn) = 0;
    virtual void onServerShutdown(Server* srv, Server::ShutdownReason reason) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCP_SERVER__ */

