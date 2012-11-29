#ifndef _EYELINE_SMPP_SOCKETMGR_H
#define _EYELINE_SMPP_SOCKETMGR_H

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "Socket.h"
#include "SmppIOTask.h"
#include <map>
#include <vector>

namespace eyeline {
namespace smpp {

enum TimerType {
        TIMER_BIND = 1,      // when to state bind failed
        TIMER_ACTIVITY = 2,  // when to send enquire link
        TIMER_CONNECT = 3    // when to state connect failed
};


inline const char* timerTypeToString( TimerType t ) {
    switch (t) {
    case TIMER_BIND : return "bind"; break;
    case TIMER_ACTIVITY : return "act"; break;
    case TIMER_CONNECT: return "conn"; break;
    default: return "???";
    }
}


class TimerSubscriber
{
protected:
    virtual ~TimerSubscriber() {}
public:
    virtual void timerElapsed( msectime_type  now,
                               Socket&        socket,
                               TimerType      type ) = 0;
};

class SocketCloser;


struct SocketMgrConfig
{
    unsigned lastActivityTimeout;  // when to tear connection on read
    unsigned inactivityTimeout;    // when to send enquirelink
    unsigned bindTimeout;          // when to tear connection on bind
    unsigned connectRetryInterval; // when to retry connect()
    unsigned receiveMaxSize;       // max size of the input package
    unsigned socketWQueueSize;      // size of the pdu output queue
    SocketMgrConfig() :
        lastActivityTimeout(20000),
        inactivityTimeout(15000),
        bindTimeout(60000),
        connectRetryInterval(2000),
        receiveMaxSize(65000),
        socketWQueueSize(1000) {}
};


/// a socket manager
/// it supports functionality: timer and set of readers/writers
class SocketMgr : protected smsc::core::threads::Thread
{
public:
    SocketMgr( const SocketMgrConfig& config,
               TimerSubscriber& subscriber,
               SocketCloser&    closer );

    ~SocketMgr();

    /// start/stop a timer thread and readers/writers
    void start();
    void stop();

    /// add a timer for an event on socket
    /// NOTE: we don't need removeTimer() method as
    /// socket will have a one timer at a time
    /// and we will keep the alarm time in the socket itself.
    void addTimer( Socket&       sock,
                   TimerType     type );

    /// remove all timers for given socket
    void removeTimers( Socket& sock );

    /// add/remove socket to reader/writer
    /// NOTE: we don't need removeSocket, as it happens automatically
    void addSocket( Socket& socket );

    inline const SocketMgrConfig& getConfig() const { return config_; }

protected:
    virtual int Execute();

private:
    struct Timer {
        SocketPtr socket;
        TimerType     type;
    };
    typedef std::multimap< msectime_type, Timer > SocketTimers;
    typedef std::vector< SmppIOTaskPtr >      SmppIOTaskList;

    smsc::logger::Logger*                     log_;
    SocketMgrConfig                           config_;
    TimerSubscriber&                          subscriber_;
    SocketCloser&                             socketCloser_;
    smsc::core::synchronization::EventMonitor mon_;
    SocketTimers                              socketTimers_;
    bool                                      stopping_;
    smsc::core::threads::ThreadPool           tp_;
    SmppIOTaskList                            readers_;
    SmppIOTaskList                            writers_;
};

}
}

#endif
