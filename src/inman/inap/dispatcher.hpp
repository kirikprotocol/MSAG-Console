#ident "$Id$"

#ifndef __SMSC_INMAN_TCAP_DISPATCHER__
#define __SMSC_INMAN_TCAP_DISPATCHER__

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "inman/inap/inss7util.hpp"
#include "inman/inap/session.hpp"

using smsc::logger::Logger;
using smsc::core::threads::Thread;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::EventMonitor;
using smsc::inman::inap::SSNSession;

namespace smsc  {
namespace inman {
namespace inap  {

//assign this before calling TCAPDispatcher::getInstance()
extern Logger * _EINSS7_logger_DFLT; //by default "smsc.inman.inap"

class SSNSession;

//TCAPDispatcher: manages SS7 stack connecton, listens for TCAP/SCCP messages
//NOTE: this is a singleton, so initialization is not thread safe
class TCAPDispatcher : /*protected*/ Thread
{
protected:
    void* operator new(size_t);
    TCAPDispatcher();
    ~TCAPDispatcher();

public:
    typedef enum { ss7None = 0, ss7INITED, ss7OPENED, ss7CONNECTED //, ss7LISTEN
    } SS7State_T;
    typedef enum { dspStopped = 0, dspListening
    } DSPStatus_t;

    //Initializes TCAPDispatcher, returns NULL if SS7 stack is unavailable
    static TCAPDispatcher* getInstance();

    Logger * TCAPLogger(void) const { return logger; }

    //Returns true on successfull connection to SS7 stack
    //starts TCAP/SCCP message listener
    bool    connect(USHORT_T user_id, UCHAR_T toSsn);
    void    disconnect(void);
    bool    reconnect(UCHAR_T ssn);
    bool    confirmSSN(UCHAR_T ssn, UCHAR_T bindResult);
    /* TCAP/SCCP message listener methods */
    void    Stop();     //stops  thread that listens for TCAP/SCCP messages
    void    onDisconnect(void);

    SS7State_T  getState(void) const { return state; }

    //Opens or reinitializes SSNSession (TCAP dialogs factory)
    SSNSession*  openSession(UCHAR_T ssn, const char* own_addr,
                     const char* rmt_addr, ACOID::DefinedOIDidx dialog_ac_idx, USHORT_T max_id);
    SSNSession*  openSession(UCHAR_T ssn, const char* own_addr, UCHAR_T rmt_ssn,
                     const char* rmt_addr, ACOID::DefinedOIDidx dialog_ac_idx, USHORT_T max_id);
    SSNSession* findSession(UCHAR_T ssn);

protected:
    int  Execute();         //Listener thread entry point
    int  Listen();          //listens for TCAP/SCCP messages
    
    int  connectCP(SS7State_T upTo = ss7CONNECTED);   //Returns:  (-1) - failed to connect,
                            //0 - already connected, 1 - successfully connected
    void disconnectCP(SS7State_T downTo = ss7None);

    bool bindSSN(UCHAR_T ssn);
    void bindSSNs(void);
    void unbindSSNs(void);
    unsigned unbindedSSNs(void);


    typedef std::map<UCHAR_T, SSNSession*> SSNmap_T;
    EventMonitor    _mutex;
    Event           lstEvent;
    USHORT_T        userId;
    SSNmap_T        sessions;
    SS7State_T      state;
    volatile DSPStatus_t   _status;
    time_t          lastBindReq;
    Logger*         logger;
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCAP_DISPATCHER__ */

