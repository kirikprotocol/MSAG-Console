#ident "$Id$"
#ifndef __SMSC_INMAN_TCP_SERVER__
#define __SMSC_INMAN_TCP_SERVER__

#include <string>
#include <list>

#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "inman/interaction/connect.hpp"

using smsc::core::threads::Thread;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::Event;
using smsc::inman::interaction::Connect;

namespace smsc  {
namespace inman {
namespace interaction  {

typedef struct {
    std::string     host;
    int             port;
    unsigned int    maxConn;
    unsigned int    timeout; //units: secs
} ServSocketCFG;

class ServerListener;
class Server : Thread, public ObservableT< ServerListener > {
public:
    typedef enum { lstStopped = 0, lstStopping, lstRunning } ServerState;
    typedef enum {
        srvUnexpected = -1, //listener caught unexpected fatal exception
        srvStopped = 0,     //normal shutdown
        srvError = 1        //server socket fatal error
    } ShutdownReason;

    Server(const ServSocketCFG * in_cfg, Logger* uselog = NULL);
    virtual ~Server();

    void openConnect(Connect* connect);
    void closeConnect(Connect* connect, bool abort = false);

    bool Start(void);
    void Stop(unsigned int timeOutMilliSecs = 400);

protected:
    typedef std::list<Connect*> ConnectsList;

    int  Execute(); //listener thread entry point
    ShutdownReason Listen(void);
    //Closes all client's connections
    void closeAllConnects(bool abort = false);

    Event           lstEvent;
    Mutex           _mutex;
    ServSocketCFG   _cfg;
    volatile
        ServerState _runState;
    unsigned        lstRestartCnt;
    Socket          serverSocket;
    ConnectsList    connects;
    Logger*         logger;
};

class ServerListener {
public:
    virtual void onConnectOpened(Server* srv, Connect* conn) = 0;
    virtual void onConnectClosing(Server* srv, Connect* conn) = 0;
    virtual void onServerShutdown(Server* srv, Server::ShutdownReason reason) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCP_SERVER__ */

