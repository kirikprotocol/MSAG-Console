/* ************************************************************************** *
 * IAPQueryATSI: implements functionality for quering the HLR for various 
 *               abonent's Camel Subscription Information using
 *               ANY_TIME_SUBSCRIPTION_INTERROGATION service.
 * Able to determine abonent contract, operator determined barrings,
 * gsmSCF parameters (address & serviceKey) 
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPQUERY_HLR_ATSI_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAPQUERY_HLR_ATSI_HPP

#include "inman/common/OptionalObjT.hpp"
#include "inman/common/IDXObjPool_T.hpp"
#include "inman/abprov/facility2/inc/IAPQuery.hpp"
#include "inman/inap/map_atih/DlgMapATSI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace atih {

using smsc::inman::iaprvd::IAPQueryAC;

using smsc::inman::inap::ObjFinalizerIface;
using smsc::inman::inap::ObjAllcStatus_e;

using smsc::inman::inap::TCSessionMA;
using smsc::inman::inap::atih::MapATSIDlg;
using smsc::inman::inap::atih::ATSIhandlerITF;

using smsc::inman::comp::UnifiedCSI;
using smsc::inman::comp::atih::RequestedSubscription;
using smsc::inman::comp::atih::ATSIRes;

/* ************************************************************************** *
 * 
 * ************************************************************************** */
class IAPQueryATSI_CFG  : public RequestedSubscription {
public:
  uint16_t        _tmoSecs;
  TCSessionMA *   _mapSess;

  IAPQueryATSI_CFG()
    : RequestedSubscription(UnifiedCSI::csi_O_BC)
    , _tmoSecs(20), _mapSess(NULL)
  {
    setRequestedODB();
  }
  IAPQueryATSI_CFG(uint16_t tmo_secs, TCSessionMA & map_sess)
    : RequestedSubscription(UnifiedCSI::csi_O_BC)
    , _tmoSecs(tmo_secs), _mapSess(&map_sess)
  {
    setRequestedODB();
  }
  ~IAPQueryATSI_CFG()
  { }
};

class IAPQueryATSI : public IAPQueryAC, protected ATSIhandlerITF {
private:
  IAPQueryATSI_CFG                      _cfg;
  smsc::util::OptionalObj_T<MapATSIDlg> _mapDlg;

public:
  static const TypeString_t _qryType; //"IAPQueryATSI"

  explicit IAPQueryATSI(IAPQueryId q_id) : IAPQueryAC(q_id)
  {
    mkTaskName();
  }

  void configure(const IAPQueryATSI_CFG & use_cfg) { _cfg = use_cfg; }
  // -------------------------------------------------------
  // -- IAPQueryAC interface methods
  // -------------------------------------------------------
  virtual const TypeString_t & taskType(void) const /*throw()*/ { return _qryType; }
  //Starts query execution. May be called only at qryIdle state. 
  //In case of success switches FSM to qryStarted state,
  //otherwise to qryDone state.
  virtual IAPQStatus::Code  start(Logger * use_log = NULL) /*throw()*/;
  //Cancels query execution, tries to switche FSM to qryStopping/qryDone state.
  //Note: Listeners aren't notified.
  //Returns false if listener is already targeted and query
  //waits for its mutex.
  virtual bool              cancel(void) /*throw()*/;
  //Returns true if query object may be released.
  //Note: should be at least equal to isCompleted()
  virtual bool              isToRelease(void) /*throw()*/;
  //Releases all used resources. May be called only at qryDone state.
  //Switches FSM to qryIdle state.
  virtual void              cleanup(void) /*throw()*/;

protected:
  virtual ~IAPQueryATSI()
  { }

  friend class smsc::inman::inap::atih::MapATSIDlg;
  // -------------------------------------------------------
  // -- ATSIhandlerITF implementation:
  // -------------------------------------------------------
  virtual void onATSIResult(ATSIRes & arg);
  //if err_code != 0, no result has been got from MAP service,
  //NOTE: dialog user may destroy reporting MAP dialog from this callback in two ways:
  //   1) by calling MapDialogAC::releaseThis() if dialog is allocated on heap
  //   2) by calling ObjFinalizerIface::finalizeObj() and then MapDialogAC destructor
  virtual ObjAllcStatus_e onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash err_code = 0);
  //
  virtual void Awake(void) { notify(); }
};

/* ************************************************************************** *
 * 
 * ************************************************************************** */
class IAPQueriesPoolATSI : public IAPQueriesPoolIface {
protected:
  typedef smsc::util::IDXObjPool_T<IAPQueryATSI, IAPQueryId>  QueriesPool;

  mutable Mutex     _sync;
  QueriesPool       _objPool; //objects are reusable
  IAPQueryATSI_CFG  _cfg;

public:
  IAPQueriesPoolATSI() : _objPool(false)
  { }
  virtual ~IAPQueriesPoolATSI()
  { }

  //asigns parameters to use while configuring newly allocated IAPQueries
  void init(const IAPQueryATSI_CFG & use_cfg);

  // ------------------------------------------
  // -- IAPQueriesPoolIface interface methods
  // ------------------------------------------
  virtual void          reserveObj(IAPQueryId num_obj) /*throw()*/;
  virtual IAPQueryAC *  allcQuery(void) /*throw()*/;
  virtual void          rlseQuery(IAPQueryAC & use_obj) /*throw()*/;
};

} //atih
} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPQUERY_HLR_ATSI_HPP */

