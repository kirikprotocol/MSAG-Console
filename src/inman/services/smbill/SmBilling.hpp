/* ************************************************************************** *
 * Billing: implements SM/USSD billing process logic:
 * 1) determination of abonent contract type by quering abonent provider
 * 2) determination of IN platform and configuring its dialog parameters
 * 3) dialog with IN platform
 * 4) writing CDRs
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INAP_BILLING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_BILLING__

/* SMBilling interoperation scheme:
SMSC   < - >           Billing(SSF)     < - >  CapSMSDlg  < - >    In-platform(SCF)
ChargeSms           -> [ bilStarted,
                         bilInited:      -->   InitialDP ->  SCF]
                                                [ CapSMSDlg <- ReleaseSMS(SCF)
                                                  | CapSMSDlg <- ContinueSMS(SCF) ]
                       [ bilReleased     <--   onDPSMSResult()
ChargeSmsResult     <-   | bilProcessed ]

[ CHARGING_POSSIBLE:
  DeliverySmsResult -> [  bilApproved:   -->   eventReportSMS  --> SCF
                          bilComplete :  <--   onEndCapDlg() ]
]
*/

#include "inman/interaction/asyncmgr/AsynWorkerDefs.hpp"
#include "inman/interaction/msgbill/SmBillRequestMsg.hpp"
#include "inman/services/smbill/SmBillDefs.hpp"
#include "inman/services/smbill/CAPSmTask.hpp"

#include "inman/INManErrors.hpp"

namespace smsc {
namespace inman {
namespace smbill {

using smsc::core::synchronization::Condition;

using smsc::core::timers::TimerHdl;

using smsc::inman::INManErrorId;
using smsc::inman::ScheduledTaskAC;

using smsc::inman::iaprvd::IAPAbility;
using smsc::inman::iaprvd::AbonentId;
using smsc::inman::iaprvd::AbonentSubscription;

using smsc::inman::iapmgr::IAPRule;
using smsc::inman::iapmgr::IAPPrio_e;
using smsc::inman::iapmgr::AbonentPolicy;

using smsc::inman::comp::CSIUid_e;

using smsc::inman::interaction::SMCAPSpecificInfo;
using smsc::inman::interaction::WorkerGuard;

using smsc::inman::interaction::PacketBufferAC;
using smsc::inman::interaction::SmBillRequestMsg;
using smsc::inman::interaction::SPckChargeSmsResult;

using smsc::inman::cdr::CDRRecord;


class Billing : public smsc::inman::interaction::WorkerIface
              , protected smsc::inman::iaprvd::IAPQueryListenerITF
              , protected smsc::core::timers::TimerListenerIface
              , protected smsc::inman::TaskRefereeITF {
public:
  enum BillingState {
    bilIdle = 0,
    bilStarted,     // SSF <- SMSC : CHARGE_SMS_TAG
                    //   [Timer] SSF -> IAProvider
    bilQueried,     // SSF <- IAProvider: query result
    bilInited,      // [Timer] SSF -> SCF : InitialDPSMS
    bilReleased,    // SSF <- SCF : ReleaseSMS
                    //   SSF -> SMSC : CHARGE_SMS_RESULT_TAG(No)
    bilContinued,   // SSF <- SCF : ContinueSMS
                    //   [Timer] SSF -> SMSC : CHARGE_SMS_RESULT_TAG(Ok)
    bilAborted,     // SSF <- SMSC/Billing : Abort
    bilSubmitted,   // SSF <- SMSC : DELIVERY_SMS_RESULT_TAG
                    //   [Timer] SSF -> SCF : EventReportSMS
    bilReported,    // SSF <- SCF: T_END{OK|NO}
    bilComplete     // 
  };

  enum PGraphState {  //billing processing graph state
    //pgAbort = -1,   //processing has aborted (worker aborted itself)
    pgCont = 0,     //processing continues (negotiation with
                    //client or external module is expected)
    pgEnd = 1       //processing has been finished (worker may be released)
  };

  static const char * state2Str(BillingState bil_state);

  Billing() : smsc::inman::interaction::WorkerIface()
    , _cfg(0), _pState(bilIdle), _chrgFlags(0), _billPrio(0)
    , _msgType(BillModes::msgUnknown), _billMode(BillModes::billOFF)
    , _curIAPrvd(AbonentPolicy::iapNone)
    , _cfgScf(0), _xsmsSrv(0), _billErr(0), _capTask(0), _capSched(0)
  { }
  //
  virtual ~Billing();

  void configure(const SmBillingCFG & use_cfg, const char * id_pfx);
  //Note: presence of 'pck_buf' means input PDU is only partially deserialized
  void wrkHandlePacket(SmBillRequestMsg & recv_pck, PacketBufferAC * pck_buf = NULL) /*throw()*/;

  // ------------------------------------------
  // -- WorkerIface interface methods:
  // ------------------------------------------
  //Initializes worker as it's got from pool.
  virtual void wrkInit(smsc::inman::interaction::WorkerID w_id,
                       smsc::inman::interaction::WorkerManagerIface * use_mgr,
                       Logger * use_log = NULL);
  //Prints some information about worker state/status
  virtual void wrkLogState(std::string & use_str) const;
  //
  virtual void wrkAbort(const char * reason = NULL);

private:
  //Ids of external entities, which may refer this object
  //Note: references by TimerWatcher facility is handled in specific way!
  enum RefEntities_e {
    refIdItself = 0x0
    , refIdIAProvider = 0x01
    , refIdCapSmTask = 0x02
    , refIdMAX = 0x03 //just a max cap
  };

  struct TimerInfo {
    BillingState  mFsmState;
    TimerHdl      mSwHdl;
    WorkerGuard   mWrkGrd;

    TimerInfo() : mFsmState(bilIdle)
    { }
    //
    bool empty(void) const { return mSwHdl.empty() && (mFsmState == bilIdle); }
    //
    void clear(void) { mFsmState = bilIdle; mSwHdl.release(); mWrkGrd.release(); }
  };

  class TimersRegistry {
  protected:
    static const unsigned k_maxTimersNum = 4;

    TimerInfo   mReg[k_maxTimersNum];

  public:
    TimersRegistry()
    { }
    ~TimersRegistry()
    { }

    unsigned size(void) const { return k_maxTimersNum; }

    //returns null if no timer with given UId exists.
    TimerInfo * find(TimerUId sw_uid)
    {
      for (unsigned i = 0; i < k_maxTimersNum; ++i) {
        if (mReg[i].mSwHdl.isTimerUId(sw_uid))
          return &mReg[i];
      }
      return NULL;
    }

    //returns null if no timer at given FSM state exists.
    TimerInfo * find(BillingState fsm_state)
    {
      for (unsigned i = 0; i < k_maxTimersNum; ++i) {
        if (mReg[i].mFsmState == fsm_state)
          return &mReg[i];
      }
      return NULL;
    }

    //returns false if a timer at given FSM state already/still exists
    bool alloc(BillingState fsm_state, TimerInfo * & p_inf)
    {
      if ((p_inf = find(fsm_state))) //check uniqueness
        return false;

      if ((p_inf = find(bilIdle)))
        p_inf->mFsmState = fsm_state;
      return true;
    }
    //
    void erase(BillingState fsm_state)
    {
      TimerInfo * pInf = find(fsm_state);
      if (pInf)
        pInf->clear();
    }

    TimerInfo & operator[](unsigned use_idx) { return mReg[use_idx]; }
  };

  struct ThisGuard {
    WorkerGuard _wrkGrd;
    uint16_t    _refCnt;

    ThisGuard() : _refCnt(0)
    { }
  };

  /* - */
  mutable EventMonitor    _sync;
  const SmBillingCFG *    _cfg;     //
  volatile BillingState   _pState;
  //prefix for logging info
  char _logId[sizeof("Billing[%u:%u]") + sizeof(smsc::inman::interaction::WorkerID)*3 + 1];

  CDRRecord             _cdr;       //data for CDR record creation & CAP3 interaction
  SMCAPSpecificInfo     _csInfo;    //data for CAP3 interaction
  uint8_t               _chrgFlags; //flags which customize billing settings
  const BillModesPrio * _billPrio;  //billing modes priority 
  BillModes::MsgType_e  _msgType;   //
  BillModes::BillType_e _billMode;  //current billing mode
  // ..
  ThisGuard             _wrkRefs[refIdMAX];
  TimersRegistry        _timers;    //active timers
  AbonentSubscription   _abCsi;     //CAMEL subscription info of abonent is to charge
  TonNpiAddress         _abNumber;  //ISDN number of abonent is to charge
  IAPRule               _iapRule;   //abonent policy rule
  IAPPrio_e             _curIAPrvd;  //UId of last IAProvider asked
  // ...
  const INScfCFG *      _cfgScf;    //serving gsmSCF(IN-point) configuration
  const XSmsService *   _xsmsSrv;   //optional SMS Extra service config.
  RCHash                _billErr;   //global error code made by URCRegistry
  // ...
  TaskId                _capTask;   //id of started CapSMS task
  TaskSchedulerITF *    _capSched;  //CapSMS task scheduler
  std::string           _capName;   //CapSMS task name for logging
  Condition             _capEvent;

  /* - */
  const char * state2Str(void) { return state2Str(_pState); }

  bool hasRef(RefEntities_e ref_id) const
  {
    return _wrkRefs[ref_id]._wrkGrd.get() != NULL;
  }
  void addRef(RefEntities_e ref_id)
  {
    if (!_wrkRefs[ref_id]._refCnt)
      _wrkRefs[ref_id]._wrkGrd = _wrkMgr->getWorkerGuard(*this);
    ++_wrkRefs[ref_id]._refCnt;
  }
  void unRef(RefEntities_e ref_id)
  {
    if (_wrkRefs[ref_id]._refCnt) {
      if (!(--_wrkRefs[ref_id]._refCnt))
        _wrkRefs[ref_id]._wrkGrd.release();
    }
  }

  uint32_t getServiceKey(CSIUid_e csi_id) const;

  //Returns true if next configured IAProvider has specified ability.
  bool nextIAProviderHas(IAPAbility::Option_e op_val) const;
  //Returns true if qyery is started, so execution will continue in another thread.
  bool startIAPQuery(void);
  void cancelIAPQuery(void);
  //
  RCHash startCAPSmTask(void);
  bool unrefCAPSmTask(bool wait_report = true);
  void abortCAPSmTask(bool wait_report = true);
  //
  bool startTimer(const TimerFact & tmo_hdl);
  void stopTimer(TimerInfo & p_tmr);
  void stopTimer(BillingState bilState);

  //Returns false if PDU contains invalid data preventing request processing
  bool verifyChargeSms(void);
  PGraphState chargeResult(bool do_charge, RCHash last_err = 0);
  const AbonentPolicy * determinePolicy(void);
  void configureMOSM(void);
  INManErrorId::Code_e configureSCF(void);
  PGraphState configureSCFandCharge(void);
  void onSubmitReport(RCHash scf_err, bool in_billed = false);
  //
  bool sendCmd(const SPckChargeSmsResult & use_res) const;
  //
  unsigned writeCDR(void);
  //returns true if required CDR data fullfilled
  bool     isCDRComplete(void) const;
  //returns true if all billing stages are completed
  bool     isBillComplete(void) const;

  void doCleanUp(void);
  //returns true if this worker finalized and relaased all resources
  bool doFinalize(void);
  void abortThis(const char * reason = NULL);

protected:
  // -------------------------------------------------------
  // -- IAPQueryListenerITF interface methods:
  // -------------------------------------------------------
  //Returns false if listener unable to handle query report right now, so
  //requests query to be rereported.
  virtual bool onIAPQueried(const AbonentId & ab_number,
                            const AbonentSubscription & ab_info, RCHash qry_status);
  // -------------------------------------------------------
  // -- TimerListenerIface interface methods:
  // -------------------------------------------------------
  virtual EventResult_e onTimerEvent(TimerUId tmr_id, const char * tmr_stat);
  // -------------------------------------------------------
  // -- TaskRefereeITF interface methods:
  // -------------------------------------------------------
  virtual void onTaskReport(TaskSchedulerITF * sched, const ScheduledTaskAC * task);
  // -------------------------------------------------------
  // -- INPBillingHandlerITF analogous methods:
  // -------------------------------------------------------
  PGraphState onChargeSms(void);
  PGraphState onDeliverySmsResult(void);
};

} //smbill
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

