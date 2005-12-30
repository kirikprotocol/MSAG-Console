#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_DISPATCHER__
#define __SMSC_INMAN_INAP_DISPATCHER__

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Mutex.hpp"

#include "inman/inap/inss7util.hpp"
#include "inman/inap/session.hpp"

using smsc::logger::Logger;
using smsc::core::threads::Thread;
using smsc::core::synchronization::Mutex;
using smsc::inman::inap::Session;

namespace smsc  {
namespace inman {
namespace inap  {

//assign this before calling TCAPDispatcher::getInstance()
extern Logger * _EINSS7_logger_DFLT; //by default "smsc.inman.inap"

//class Session;

//TCAPDispatcher: manages SS7 stack connecton, listens for TCAP/SCCP messages
//maintains factory of TCAP sessions (1 session per SSN)
//NOTE: this is a singleton, so initialization is not thread safe
class TCAPDispatcher : public Thread
{
protected:
    void* operator new(size_t);
    TCAPDispatcher();
    ~TCAPDispatcher();

public:
    typedef enum {
        ss7None, ss7INITED, ss7OPENED, ss7CONNECTED, ss7LISTEN
    } SS7State_T;

    //Initializes TCAPDispatcher, returns NULL if SS7 stack is unavailable
    static TCAPDispatcher* getInstance();

    Logger * TCAPLogger(void) const { return logger; }

    //Returns true on successfull connection to SS7 stack
    bool    connect(USHORT_T user_id = MSG_USER_ID, SS7State_T upTo = ss7CONNECTED);
    void    disconnect(void);
    bool    reconnect(SS7State_T upTo = ss7CONNECTED);

    /* TCAP sessions factory methods */
    Session* openSession(UCHAR_T ssn, const char* ownaddr,
                         const char* remoteaddr);
    Session* openSession(UCHAR_T ownssn, const char* ownaddr,
                         UCHAR_T remotessn, const char* remoteaddr);
    bool     confirmSession(UCHAR_T ssn, UCHAR_T bindResult);

    Session* findSession(UCHAR_T ssn);
    void     closeSession(Session* sess);
    void     closeAllSessions(void);

    /* TCAP/SCCP message listener methods */

    //void Start();  //starts thread that listens for TCAP/SCCP messages
    void Stop();     //stops  thread that listens for TCAP/SCCP messages
    int  Execute();  //Listener thread entry point
        
    SS7State_T  getState(void) const { return state; }

protected:
    int Listen(); //listens for TCAP/SCCP messages

    typedef std::map<UCHAR_T, Session*> SessionsMap_T;

    Mutex           _mutex;
    volatile bool   running;
    SessionsMap_T   sessions;
    SS7State_T      state;
    USHORT_T        userId;
    Logger*         logger;
};


/*
class Dispatcher : public Thread
{
    public:
        Dispatcher();
        virtual ~Dispatcher();

        virtual void Run();
        virtual void Stop();
        virtual int  Execute();


    protected:
        Event           started;
        Event           stopped;
        volatile bool   running;
        Logger*         logger;
};
*/
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_DISPATCHER__ */

