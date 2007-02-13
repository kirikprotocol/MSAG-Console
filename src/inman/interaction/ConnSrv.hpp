#ident "$Id$"
/* ************************************************************************** *
 * Sockets listening server: checks registered sockets for incoming data and
 * calls ConnectAC methods.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_CONN_SERVER__
#define __SMSC_INMAN_CONN_SERVER__

#include <string>
#include <map>

#include "core/threads/Thread.hpp"
using smsc::core::threads::Thread;

#include "core/synchronization/Event.hpp"
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::Event;

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "inman/interaction/ConnectDefs.hpp"
using smsc::inman::interaction::ConnectAC;

namespace smsc  {
namespace inman {
namespace interaction  {

class ConnectSrv : Thread {
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
    
    unsigned    addConnection(ConnectAC * use_conn); //Retuns conn_id
    ConnectAC * rlseConnection(unsigned conn_id);

    Socket * setConnection(const char * host, unsigned port, unsigned timeout_secs = 5);

    bool     Start(void);
    void     Stop(unsigned timeOut_msecs = 400);
    SrvState State(void);

protected:
    typedef std::map<unsigned, ConnectAC*> ConnectsMap;

    int  Execute(); //listener thread entry point
    ShutdownReason Listen(void);
    void closeConnect(ConnectAC* connect, bool abort = false);
    void closeAllConnects(bool abort = false);

    Event           lstEvent;
    Mutex           _mutex;
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

