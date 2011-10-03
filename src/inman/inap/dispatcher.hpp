/* ************************************************************************* *
 * TCAP Dispatcher types, interfaces and helpers definitions
 * ************************************************************************* */
#ifndef __INMAN_TCAP_DISPATCHER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCAP_DISPATCHER_HPP

#include "core/buffers/LWArrayTraitsInt.hpp"
#include "core/buffers/LWArrayT.hpp"

#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "inman/inap/TCDspDefs.hpp"
#include "inman/inap/TCDspIface.hpp"
#include "inman/inap/HDSSnSession.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

//using smsc::core::synchronization::EventMonitor;

//TCAPDispatcher: manages SS7 stack connecton, listens for TCAP messages
//NOTE: this is a singleton, so initialization is not thread safe
class TCAPDispatcher : public TCAPDispatcherITF
                      , protected smsc::core::threads::Thread {
private:
    using smsc::core::threads::Thread::Start; //is overloaded

    class MessageListener : smsc::core::threads::Thread {
    private:
      using smsc::core::threads::Thread::Start; //is overloaded
      // ----------------------------
      // -- Thread interface methods
      // ----------------------------
      int  Execute(void);  //thread entry point

    protected:
      mutable smsc::core::synchronization::EventMonitor  _sync;
      volatile bool         _running;
      TCAPDispatcher &      _dsp;

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

    //typedef std::map<uint8_t, SSNSession *> SSNmap_T;

    struct SSNInfo {
      SSNSession * pSess;

      explicit SSNInfo(SSNSession * use_ptr = 0) : pSess(use_ptr)
      { }

      bool operator< (const SSNInfo & cmp_obj) const
      {
        return (pSess && cmp_obj.pSess) ? (pSess->getSSN() < cmp_obj.pSess->getSSN())
                                        : pSess < cmp_obj.pSess;
      }
    };
    
    class SSNmap_T {  //sorted array
    protected:
      typedef smsc::core::buffers::LWArray_T<
        SSNInfo, uint8_t, 10, smsc::core::buffers::LWArrayTraitsPOD_T
      > SSNArray;
      
      SSNArray  mArr;

      SSNArray::size_type findPos(uint8_t ssn_id) const
      {
        if (!mArr.empty()) {
          size_type atIdx = ((mArr.size() - 1) >> 1); //approximately middle position
          if (mArr.get()[atIdx].pSess->getSSN() == ssn_id)
            return atIdx;

          if (mArr.get()[atIdx].pSess->getSSN() < ssn_id) { //go to the end
            while (++atIdx < mArr.size()) {
              if (mArr.get()[atIdx].pSess->getSSN() == ssn_id)
                return atIdx;
            }
          } else {  //go to the start
            while (atIdx) {
              if (mArr.get()[--atIdx].pSess->getSSN() == ssn_id)
                return atIdx;
            }
          }
        }
        return mArr.npos();
      }

    public:
      typedef uint8_t size_type;

      SSNmap_T()
      { }
      ~SSNmap_T()
      { }

      size_type npos(void) const { return mArr.npos(); }

      bool empty(void) const { return mArr.empty(); }
      size_type size(void) const { return mArr.size(); }

      //Returns npos() in case of failure
      size_type insert(SSNSession * p_sess) { return mArr.insert(SSNInfo(p_sess)); }
      //
      void erase(size_type at_idx) /*throw()*/
      {
        if (at_idx < mArr.size())
          mArr.erase(at_idx, 1);
      }

      SSNSession * find(uint8_t ssn_id) const
      {
        size_type atIdx = findPos(ssn_id);
        return (atIdx == mArr.npos()) ? 0 : mArr.get()[atIdx].pSess;
      }
      //
      SSNSession * operator[] (size_type at_idx) const /*throw()*/
      {
        return (at_idx < mArr.size()) ? mArr.get()[at_idx].pSess : 0;
      }
    };

    mutable smsc::core::synchronization::EventMonitor _sync;
    volatile SS7State_e  _ss7State;
    volatile DSPState_e  _dspState;
    volatile unsigned    _connCounter;

    TCDsp_CFG       _cfg;
    unsigned        _maxUConnAtt;
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
    DSPState_e  Stop(bool do_wait = false);

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

