/* ************************************************************************** *
 * TCProvider: TCAP dialogue finite state machine.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLG_FSM_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_TDLG_FSM_HPP

#include "logger/Logger.h"
#include "core/synchronization/EventMonitor.hpp"

#include "eyeline/util/SFPointer.hpp"

#include "eyeline/tcap/provd/TDlgConfig.hpp"
#include "eyeline/tcap/provd/TDlgCoordinatorIface.hpp"
#include "eyeline/tcap/provd/TDlgIndComposers.hpp"
#include "eyeline/tcap/provd/TDlgReqComposers.hpp"
#include "eyeline/tcap/provd/TCompIndComposers.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::EventMonitor;

using eyeline::corex::timers::TimerIdent;
using eyeline::corex::timers::OpaqueArg;
using eyeline::corex::timers::TimeWatcherIface;

using eyeline::tcap::TCRCode_e;
using eyeline::tcap::TComponentsPtrList;
using eyeline::tcap::TComponentsList;

struct TComponentType {
  enum Kind_e {
    compAny = 0x00
    , compInvokeOnly = 0x01
    , compNotReturnable = 0x02
  };
};

typedef TComponentType::Kind_e TComponentType_e;

typedef smsc::core::synchronization::SFPointer_T<TDlgIndHandlerIface>       TDlgIndHandlerPtr_t;
typedef smsc::core::synchronization::SFPointerGuard_T<TDlgIndHandlerIface>  TDlgIndHandlerGuard;


class ObjRefGuard {
private:
  ObjRefGuard(const ObjRefGuard & cp_obj);
  void operator=(const ObjRefGuard & cp_obj);

protected:
  mutable Mutex   _sync;
  unsigned        _refCnt;

public:
  ObjRefGuard() : _refCnt(0)
  { }
  ~ObjRefGuard()
  { }

  //Returns number of references to designated object
  unsigned RefCount(void) const
  {
    MutexGuard grd(_sync);
    return _refCnt;
  }
  //Increases references counter
  //NOTE: Returns zero in case of references counter overloading!
  unsigned AddRef(void)
  {
    MutexGuard grd(_sync);
    return ++_refCnt;
  }
  //Decreases references counter, utilizes designated object
  //if no more references exist
  unsigned UnRef(void)
  {
    MutexGuard grd(_sync);
    if (_refCnt)
        --_refCnt;
    return _refCnt;
  }
};


/* ************************************************************************** *
 * TCAP structured dialogue State Machine
 * ************************************************************************** */
class TDialogueFSM : public smsc::core::synchronization::EventMonitor,
                     protected eyeline::corex::timers::TimerListenerIface {
public:
  enum TransactionState_e {
      fsmIDLE = 0       //dialogue is idle //or finished
    , fsmINIT_SENT      //dialogue association requested
    , fsmINIT_RECEIVED  //dialogue association request received
    , fsmACTIVE         //dialogue association is established
    , fsmDONE           //dialogue is finished
  };

protected:
  static const uint8_t _DFLT_INVOCATIONS_NUM = 4;

  typedef eyeline::corex::timers::TimerHdl    TimerHdl;

  enum TimerState_e {
      tmoIdle = 0
    , tmoActive     //timer is active, reference to dialogueFSM object presents
    , tmoCancelled  //timer is cancelled, reference to dialogueFSM object presents
    , tmoDone       //timer is finished, no reference to dialogueFSM object exists
  };

  struct TimerInfo {
    TimerState_e  _state;
    TimerHdl      _hdl;
    TimerIdent    _uid;

    TimerInfo() : _state(tmoIdle)
    { }
    ~TimerInfo()
    { }

    bool empty(void) const    { return _hdl.empty(); }
    bool isActive(void) const { return _state == tmoActive; }
    bool isIdle(void) const   { return _state == tmoIdle || _state == tmoDone; }
    void clear(void)          { _state = tmoIdle;  _uid.clear();  }
  };

  //NOTE: position in array is a associated Invokaction ids, which are assigned incrementally !
  typedef smsc::core::buffers::LWArray_T<TimerInfo, uint8_t, _DFLT_INVOCATIONS_NUM> LocalInvokes;

  //Sorted array of remote Invocation Ids. (position != invId)
  typedef smsc::core::buffers::LWArray_T<uint8_t, uint8_t, _DFLT_INVOCATIONS_NUM>   RemoteInvokes;

  typedef std::list<TC_LRejectIndComposer>  TCRejectList;

  /* -- */
  mutable EventMonitor    _sync;
  ObjRefGuard &           _refGrd;
  TDlgConfig              _cfg;
  TDlgIndHandlerPtr_t     _dlgUser;
  TDlgCoordinatorIface *  _dlgMgr;
  TransactionId           _trnUId; //NOTE: _localTrId is a TCAP dialogueId
  TransactionState_e      _trnState;
  sccp::SCCPAddress       _rmtAddr;

  uint8_t                 _nextInvId;
  TimerInfo               _dlgTmo;
  LocalInvokes            _lclInv;
  RemoteInvokes           _rmtInv;
  TCRejectList            _rejComps;  //last indication rejected components info

  char                    _logId[sizeof("TDlg[%u]")+sizeof(TDialogueId)*3];
  Logger *                _logger;
  mutable Mutex           _lockHandler; //guards indications handler access

  /* -- */
  void log_error_state(const char * tr_prim) const;
  bool stopTimer(TimerInfo & tmo_info);
  bool startInvokeTimer(uint8_t inv_id, bool is_restart = false) /*throw()*/;
  bool activateDialogueTimer(void) /*throw()*/;
  bool activateInvokeTimer(uint8_t inv_id, uint16_t tmo_secs = 0) /*throw()*/;
  bool cancelInvokeTimer(uint8_t inv_id) /*throw()*/;
  bool activateTCompTimers(const TComponentsPtrList & comp_list) /*throw()*/;
  void cancelTCompTimers(const TComponentsPtrList & comp_list,
                        TComponentsPtrList::const_iterator end_it) /*throw()*/;

  //Cancels all timers initiated by dialogue
  void cancelDialogueTimers(void) /*throw()*/;

  //Verifies that components list is to send contains allowed components
  //with correct invocation Ids.
  TCRCode_e
    verifyTCReqComponents(const TComponentsPtrList & comp_list, const char * req_id,
                          const TComponentType_e comp_type) const /*throw()*/;
  //Verifies that received components list contains allowed components,
  //searches for invocation results/errors and stops associated invocation timers.
  //Retuns false if there are at least one component is to reject
  bool verifyTCIndComponents(TComponentsList * comps, const char * tind_id,
                             const TComponentType_e comp_type) /*throw()*/;

  //Searches received component list for invocation results/errors and stops
  //associated invocation timers.
  void registerInvocationResults(const TComponentsList * comps) /*throw()*/;

  //Returns false if TC User generates exception while indication handling.
  //NOTE: TDialogueFSM and TCUser must be locked upon entry!!!
  template<class TDLG_IND_COMPOSER>
    bool notifyLocalTCUser(TDlgIndHandlerGuard & grd_user, TDLG_IND_COMPOSER & tr_ind) /* throw() */;

  void stopDialogue(void) /*throw()*/;

  // ---------------------------------------
  // -- TimerListenerIface implementaton
  // ---------------------------------------
  virtual TimeWatcherIface::SignalResult_e
    onTimerEvent(const TimerIdent & tm_uid, const OpaqueArg * opaque_arg = NULL);

public:
  TDialogueFSM(ObjRefGuard & use_grd)
    : _refGrd(use_grd), _dlgMgr(NULL), _trnState(fsmIDLE), _nextInvId(0)
  {
    _logId[0] = 0;
  }
  ~TDialogueFSM()
  { }

  void init(TDialogueId dlg_id, const TDlgLocalConfig & dlg_cfg,
              TDlgCoordinatorIface * dlg_mgr);
  void init(TDialogueId dlg_id, const TDlgRemoteConfig & dlg_cfg,
              TDlgCoordinatorIface * dlg_mgr);

  //associates with remote transaction
  void bindRemote(uint32_t rmt_tr_id, const sccp::SCCPAddress & rmt_addr)
  {
    _trnUId.bindRemote(rmt_tr_id);
    _rmtAddr = rmt_addr;
  }

//  void setSCSPLinkNum(unsigned int scsp_link_num) { _cfg._scspLink = scsp_link_num; }

  unsigned AddRef(void) { return _refGrd.AddRef(); }
  void UnRef(void)      { _refGrd.UnRef(); }

  static const char* const stateToString(TransactionState_e transactionState);
  //
  const char* const stateToString(void) const { return stateToString(_trnState); }

  //
  TDialogueId           getDialogueId(void) const { return _trnUId.getIdLocal(); }
  const char * const    getLogId(void) const { return _logId; }
  const TransactionId & getTrnUId(void) const { return _trnUId; }

  const asn1::EncodedOID &  getAppContext(void) const { return _cfg._acOid; }
  const sccp::SCCPAddress & getOwnAddr(void) const { return _cfg._ownAddr; }
  const sccp::SCCPAddress & getRmtAddr(void) const { return _rmtAddr; }

  unsigned int getSCSPLinkNum(void) const;

  //-- Dialogue request primitives verification and finalization
  TCRCode_e prepareDlgRequest(TBeginReqComposer & treq_begin, SCSPUnitdataReq * req_udt) const /*throw()*/;
  TCRCode_e prepareDlgRequest(TContReqComposer & treq_cont, SCSPUnitdataReq * req_udt) const /*throw()*/;
  TCRCode_e prepareDlgRequest(TEndReqComposer & treq_end, SCSPUnitdataReq * req_udt) const /*throw()*/;
  TCRCode_e prepareDlgRequest(TUAbortReqComposer & treq_uAbort, SCSPUnitdataReq * req_udt) const /*throw()*/;

  // -- Dialogue(Component contained) request primitives handling
  TCRCode_e updateDlgByRequest(const TBeginReqComposer & treq_begin) /*throw(std::exception)*/;
  TCRCode_e updateDlgByRequest(const TContReqComposer & treq_cont) /*throw(std::exception)*/;
  TCRCode_e updateDlgByRequest(const TEndReqComposer & treq_end) /*throw(std::exception)*/;
  TCRCode_e updateDlgByRequest(const TUAbortReqComposer & treq_uAbort) /*throw(std::exception)*/;
  // -- Component only request primitives handling
  TCRCode_e updateDlgByRequest(const TC_U_Cancel_Req & tc_req) /*throw()*/;
  TCRCode_e updateDlgByRequest(const TC_Timer_Reset_Req & tc_req) /*throw()*/;

  // -- Dialogue(Component contained) handling primitives
  void updateDlgByIndication(TBeginIndComposer & tind_begin) /*throw(std::exception)*/;
  void updateDlgByIndication(TContIndComposer & tind_cont) /*throw(std::exception)*/;
  void updateDlgByIndication(TEndIndComposer & tind_end) /*throw(std::exception)*/;
  void updateDlgByIndication(TUAbortIndComposer & tind_uAbort) /*throw(std::exception)*/;
  void updateDlgByIndication(TPAbortIndComposer & tind_pAbort) /*throw(std::exception)*/;
  void updateDlgByIndication(TNoticeIndComposer & tind_notice) /*throw(std::exception)*/;
  // -- Component only handling primitives
  void updateDlgByIndication(TC_LCancelIndComposer & tc_ind) /*throw(std::exception)*/;
};

#include "eyeline/tcap/provd/TDialogueFSM_impl.hpp"

}}}

#endif /* __EYELINE_TCAP_PROVD_TDLG_FSM_HPP */

