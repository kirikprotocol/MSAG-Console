/* ************************************************************************* *
 * TCAP Dispatcher types, interfaces and helpers definitions
 * ************************************************************************* */
#ifndef __INMAN_TCAP_DISPATCHER_HPP
#ident "@(#)$Id$"
#define __INMAN_TCAP_DISPATCHER_HPP

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "core/threads/Thread.hpp"
using smsc::core::threads::Thread;

#include "core/synchronization/EventMonitor.hpp"
using smsc::core::synchronization::EventMonitor;

#include "inman/inap/TCDspDefs.hpp"
#include "inman/inap/HDSSnSession.hpp"
using smsc::inman::inap::SSNSession;

namespace smsc  {
namespace inman {
namespace inap  {

//TCAPDispatcher: manages SS7 stack connecton, listens for TCAP/SCCP messages
//NOTE: this is a singleton, so initialization is not thread safe
class TCAPDispatcher : Thread, public TCAPDispatcherITF {
private:
    using Thread::Start; //hide it to avoid annoying CC warnings

protected:
    typedef std::map<uint8_t, SSNSession*> SSNmap_T;

    mutable EventMonitor _sync;
    volatile bool   _listening;
    TCDsp_CFG       _cfg;
    SSNmap_T        sessions;
    SS7State_T      state;
    time_t          lastBindReq;
    Logger*         logger;
    const char *    _logId; //logging prefix

    // -----------------------------------
    // Thread interface methods
    // -----------------------------------
    int  Execute(void);         //Listener thread entry point

    SSNSession* lookUpSSN(uint8_t ssn) const
    {
        SSNmap_T::const_iterator it = sessions.find(ssn);
        return (it == sessions.end()) ? NULL : it->second;
    }

    //listens for TCAP messages(indications),
    //maintain SS7 autoreconnect ability
    int  Listen(void);          

    //Returns:  (-1) - failed to connect, 0 - already connected, 1 - successfully connected
    int  connectCP(SS7State_T upTo = ss7CONNECTED);
    void disconnectCP(SS7State_T downTo = ss7None);
    bool bindSSN(SSNSession * p_session) const;
    void bindSSNs(void) const;
    void unbindSSN(SSNSession * p_session) const;
    void unbindSSNs(void) const;
    unsigned unbindedSSNs(void) const;
    //Checks for unconnected TCAP BE instances and returns its total number
    unsigned disconnectedUnits(void) const;
    //Connects currently disconnected TCAP BE instances.
    //Returns true if new instances become available (connected)
    bool connectUnits(void);
    //Disonnects all TCAP BE instances.
    void disconnectUnits(void);


public:
    TCAPDispatcher();
    ~TCAPDispatcher();

    //Initializes TCAPDispatcher and SS7 communication facility
    //Returns true on success.
    //NOTE: must be called prior to using TCAPDispatcher
    bool Init(TCDsp_CFG & use_cfg, Logger * use_log = NULL);
    //
    bool isListening(void) const
    {
        MutexGuard grd(_sync);
        return _listening;
    }

    //Returns true on successfull connection to TCAP unit of SS7 stack,
    //starts TCAP messages(indications) listener
    bool    Start(void);
    //Stops TCAP messages(indications) listener, unbinds all SSNs,
    //if do_wait is set sets ss7INITED state
    void    Stop(bool do_wait = false);

    //Notifies dispatcher that broken connection is detected by third party
    void    onDisconnect(void);
    //NOTE: This one is called only by EINSS7 TCAP API callbacks!
    void    confirmSSN(uint8_t ssn, uint8_t tc_inst_id, uint8_t bindResult);

    // -----------------------------------
    // TCAPDispatcherITF interface methods
    // -----------------------------------
    //Returns state of TCAP unit connection
    SS7State_T  ss7State(void) const
    {
        MutexGuard grd(_sync);
        return state; 
    }
    //Binds SSN and initializes SSNSession (TCAP dialogs factory)
    SSNSession *openSSN(uint8_t ssn_id, uint16_t max_dlg_id = 2000,
                        Logger * uselog = NULL);
    //
    SSNSession* findSession(uint8_t ssn) const;
    //
    ApplicationContextRegistryITF * acRegistry(void) const;
};

} //inap
} //inman
} //smsc

#endif /* __INMAN_TCAP_DISPATCHER_HPP */

