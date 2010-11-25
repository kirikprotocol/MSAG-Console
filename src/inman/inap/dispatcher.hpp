/* ************************************************************************* *
 * TCAP Dispatcher types, interfaces and helpers definitions
 * ************************************************************************* */
#ifndef __INMAN_TCAP_DISPATCHER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCAP_DISPATCHER_HPP

#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "inman/inap/TCDspDefs.hpp"
#include "inman/inap/TCDspIface.hpp"
#include "inman/inap/HDSSnSession.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

using smsc::core::threads::Thread;
using smsc::core::synchronization::EventMonitor;

//TCAPDispatcher: manages SS7 stack connecton, listens for TCAP messages
//NOTE: this is a singleton, so initialization is not thread safe
class TCAPDispatcher : Thread, public TCAPDispatcherITF {
private:
    using Thread::Start; //hide it to avoid annoying CC warnings

    class MessageListener : Thread {
    private:
      using Thread::Start; //hide it to avoid annoying CC warnings

    protected:
      mutable EventMonitor  _sync;
      volatile bool         _running;
      TCAPDispatcher &      _dsp;

      // ----------------------------
      // -- Thread interface methods
      // ----------------------------
      int  Execute(void);  //thread entry point

    public:
      MessageListener(TCAPDispatcher & use_dsp)
        : _running(false), _dsp(use_dsp)
      { }
      ~MessageListener()
      {
        Stop(true);
      }

      bool isRunning(void) const;
      bool Start(void);
      void Stop(bool do_wait = false);
      void Notify(void) const { _sync.notify(); }
    };

    typedef std::map<uint8_t, SSNSession *> SSNmap_T;

    mutable EventMonitor _sync;
    volatile SS7State_e  _ss7State;
    volatile DSPState_e  _dspState;
    volatile unsigned    _connCounter;

    TCDsp_CFG       _cfg;
    SS7Unit_CFG *   _unitCfg;   //TCAP Unit(s) configuration
#ifdef EIN_HD
    char *          _rcpMgrAdr; //remote CP Manager addresses
#else
    Mutex           _msgRecvLock;
#endif /* EIN_HD */
    SSNmap_T        _sessions;
    const char *    _logId;     //logging prefix
    Logger *        logger;
    MessageListener _msgAcq;   //Message acquiring thread

    //maintains SS7 Units autoreconnection 
    int  Reconnect(void);

    // -----------------------------------
    // -- Thread interface methods
    // -----------------------------------
    int  Execute(void);         //Autoconnection thread entry point

protected:
    SSNSession * lookUpSSN(uint8_t ssn) const
    {
        SSNmap_T::const_iterator it = _sessions.find(ssn);
        return (it == _sessions.end()) ? NULL : it->second;
    }

    //Returns:  (-1) - failed to connect, 0 - already connected, 1 - successfully connected
    int  connectCP(SS7State_e upTo = ss7CONNECTED);
    void disconnectCP(SS7State_e downTo = ss7None);

    //Checks for unconnected TCAP BE instances and returns its total number
    unsigned disconnectedUnits(bool * is_all = NULL) const;
    //Connects currently disconnected TCAP BE instances.
    //Returns number of new instances succesfully connected
    unsigned connectUnits(void);
    //Disonnects all TCAP BE instances.
    void disconnectUnits(void);

    //Check all connected unit instances for binding status
    //Returns true if at least one unit instance needs binding
    bool unitsNeedBinding(void) const;
    //Checks all SubSystems for bind status.
    //Returns number of unbided SSNs. 
    unsigned unbindedSSNs(void) const;

    //Binds SubSystem to each unit instance.
    //Returns true if at least one BindReq() is successfull
    bool bindSSN(SSNSession * p_session) const;
    //Binds all SubSystems to each unit instance.
    //Returns true if at least one BindReq() succeeded
    bool bindSSNs(void) const;
    //Unbinds SubSystem from each unit instance.
    void unbindSSN(SSNSession * p_session) const;
    //Unbinds all SubSystems from each unit instance.
    void unbindSSNs(void) const;

    friend class MessageListener;

    bool msgListeningOn(void) const;
    unsigned short dispatchMsg(void);

public:
    TCAPDispatcher();
    ~TCAPDispatcher();

    //Initializes TCAPDispatcher and SS7 communication facility
    //Returns true on success.
    //NOTE: must be called prior to using TCAPDispatcher
    bool Init(const TCDsp_CFG & use_cfg, Logger * use_log = NULL);
    //
    bool isRunning(void) const
    {
        MutexGuard grd(_sync);
        return _dspState == dspRunning;
    }

    //Returns true on successfull connection to TCAP unit of SS7 stack,
    //starts TCAP messages(indications) listener
    bool    Start(void);
    //Stops TCAP messages(indications) listener, unbinds all SSNs,
    //if do_wait is set sets ss7INITED state
    void    Stop(bool do_wait = false);

    //Notifies dispatcher that broken connection is detected, the connection
    //to given TCAP BE instance will be automatically reestablished.
    void    onDisconnect(unsigned short fromUsrID,
                         unsigned short toUsrID, unsigned char inst);
    void    onDisconnect(unsigned char inst);

    //NOTE: This one is called only by EINSS7 TCAP API callbacks!
    void    confirmSSN(uint8_t ssn, uint8_t tc_inst_id, uint8_t bindResult);

    // -----------------------------------
    // TCAPDispatcherITF interface methods
    // -----------------------------------
    //Returns dispatcher state
    DSPState_e  dspState(void) const
    {
        MutexGuard grd(_sync);
        return _dspState;
    }
    //Returns state of TCAP BE unit(s) connection
    SS7State_e  ss7State(void) const
    {
        MutexGuard grd(_sync);
        return _ss7State; 
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

