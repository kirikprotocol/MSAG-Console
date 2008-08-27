#ident "$Id$"
#ifndef __SMSC_INMAN_TCP_SERVER__
#define __SMSC_INMAN_TCP_SERVER__

#include <string>
#include <list>

#include "core/threads/Thread.hpp"
using smsc::core::threads::Thread;

#include "core/synchronization/Event.hpp"
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::Event;

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "inman/common/Observatory.hpp"
using smsc::util::GRDObservatoryOfT;

#include "inman/interaction/ConnectDefs.hpp"
using smsc::inman::interaction::ConnectAC;


namespace smsc  {
namespace inman {
namespace interaction  {

struct ServSocketCFG {
    std::string     host;
    unsigned int    port;
    unsigned int    maxConn;
    unsigned int    timeout; //units: secs

    ServSocketCFG()
    { port = maxConn = timeout = 0; }
};

class ServerListenerITF;
class Server : Thread, public GRDObservatoryOfT<ServerListenerITF> {
private:
    using Thread::Start; //hide it to avoid annoying CC warnings

public:
    static const unsigned int _SHUTDOWN_TMO_MS = 400; //millisecs

    enum ServerState { lstStopped = 0, lstStopping, lstRunning };
    enum ShutdownReason {
        srvUnexpected = -1, //listener caught unexpected fatal exception
        srvStopped = 0,     //normal shutdown
        srvError = 1        //server socket fatal error
    };

    Server(const ServSocketCFG * in_cfg, Logger* uselog = NULL);
    virtual ~Server();

    bool Start(void);
    //Stops listener: no new connect will be accepted, active ones are served for a while.
    //If timeout is not zero, then method blocks until listener will be completed.
    void Stop(unsigned int timeOutMilliSecs = _SHUTDOWN_TMO_MS);

    void setPollTimeout(unsigned long  milli_secs);

    //Ad hoc method: upon successfull socket initialization,
    //notifies listeners via onConnectOpening()
    bool setConnection(const char * host, unsigned port, unsigned timeout_secs);

protected:
    typedef std::list<ConnectAC*> ConnectsList;

    int  Execute(); //listener thread entry point
    ShutdownReason Listen(void);
    void openConnect(std::auto_ptr<Socket>& use_sock);
    void closeConnect(ConnectAC* connect, bool abort = false);
    void closeConnectGuarded(ConnectsList::iterator & it, bool abort = false);
    //Closes all client's connections
    void closeAllConnects(bool abort = false);

    Event           lstEvent;
    ServSocketCFG   _cfg;
    volatile
        ServerState _runState;
    unsigned        lstRestartCnt;
    Socket          serverSocket;
    ConnectsList    connects;
    struct timeval  tmo;
    Logger*         logger;
};

class ServerListenerITF {
public:
    //Listener creates its own ConnectAC implementation
    virtual ConnectAC* onConnectOpening(Server* srv, Socket* sock) = 0;
    //Listener shouldn't delete ConnectAC !!!
    virtual void onConnectClosing(Server* srv, ConnectAC* conn) = 0;
    virtual void onServerShutdown(Server* srv, Server::ShutdownReason reason) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCP_SERVER__ */

