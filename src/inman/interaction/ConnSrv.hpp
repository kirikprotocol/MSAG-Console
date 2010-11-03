/* ************************************************************************** *
 * Sockets listening server: checks registered sockets for incoming data and
 * calls ConnectAC methods.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_CONN_SERVER__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_CONN_SERVER__

#include <string>
#include <map>

#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"

#include "inman/interaction/ConnectDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

using smsc::core::threads::Thread;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::Mutex;
using smsc::logger::Logger;

using smsc::inman::interaction::ConnectAC;

class ConnectSupervisorITF {
protected:
    virtual ~ConnectSupervisorITF() //forbid interface destruction
    { }

public:
    //Returns true if ConnectAC should be utilized by ConnectSrv
    virtual bool onConnectClosed(ConnectAC * conn) = 0;
};

class ConnectSrv : Thread {
private:
    using Thread::Start; //hide it to avoid annoying CC warnings

public:
    static const unsigned POLL_TIMEOUT_ms  = 100; //default timeout for select(), millisecs
    typedef enum { lstStopped = 0, lstStopping, lstRunning } SrvState;
    typedef enum {
        srvUnexpected = -1, //listener caught unexpected fatal exception
        srvStopped = 0,     //normal shutdown
        srvError = 1        //socket engine fatal error
    } ShutdownReason;

    ConnectSrv(unsigned tmo_msecs = POLL_TIMEOUT_ms, Logger* uselog = NULL);
    virtual ~ConnectSrv();

    //Retuns conn_id, if 'mgr' == NULL, connect will be utilized by ConnectSrv
    unsigned    addConnection(ConnectAC * use_conn, ConnectSupervisorITF * mgr);
    ConnectAC * rlseConnection(unsigned conn_id);
    unsigned    numOfConnects(void);

    Socket * setConnection(const char * host, unsigned port, unsigned timeout_secs = 5);

    bool     Start(void);
    void     Stop(unsigned timeOut_msecs = 400);
    SrvState State(void);
    void WaitFor(void) { Thread::WaitFor(); }

protected:
    struct ConnectInfo {
        bool        ignore;
        ConnectAC * conn;
        ConnectSupervisorITF * mgr;

        ConnectInfo(ConnectAC * use_conn = NULL, ConnectSupervisorITF * use_mgr = NULL)
            : ignore(false), conn(use_conn), mgr(use_mgr)
        { }
    };
    typedef std::map<unsigned, ConnectInfo> ConnectsMap;

    int  Execute(); //listener thread entry point
    ShutdownReason Listen(void);
    void closeConnect(unsigned conn_id, bool abort = false);
    void closeAllConnects(bool abort = false);

    Event           lstEvent;
    EventMonitor    _Sync;
    unsigned int    maxConn;
    unsigned int    tmoMSecs;
    volatile SrvState _runState;
    unsigned        lstRestartCnt;
    ConnectsMap     connects;
    struct timeval  tmo;
    Logger*         logger;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_CONN_SERVER__ */

