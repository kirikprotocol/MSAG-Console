/* ************************************************************************** *
 * IAPQuerySRI: implements functionality for quering the HLR for abonent 
 *              Camel Subscription Information via SEND_ROUTING_INFO service.
 * Able to determine abonent contract, gsmSCF parameters (address & serviceKey)
 * IMSI and current VLR number.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPQUERY_HLR_SRI_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAPQUERY_HLR_SRI_HPP

#include "util/OptionalObjT.hpp"
#include "inman/abprov/facility2/inc/IAPQuery.hpp"
#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

using smsc::inman::iaprvd::IAPQueryAC;

using smsc::inman::inap::ObjFinalizerIface;
using smsc::inman::inap::ObjAllcStatus_e;
using smsc::inman::inap::TCSessionMA;
using smsc::inman::comp::chsri::CHSendRoutingInfoRes;

/* ************************************************************************** *
 * 
 * ************************************************************************** */
struct IAPQuerySRI_CFG {
  uint16_t        _tmoSecs;
  TCSessionMA *   _mapSess;

  IAPQuerySRI_CFG() : _tmoSecs(20), _mapSess(NULL)
  { }
  IAPQuerySRI_CFG(uint16_t tmo_secs, TCSessionMA & map_sess)
    : _tmoSecs(tmo_secs), _mapSess(&map_sess)
  { }
  ~IAPQuerySRI_CFG()
  { }
};

/* ************************************************************************** *
 * 
 * ************************************************************************** */
class IAPQuerySRI : public IAPQueryAC
                  , protected smsc::inman::inap::chsri::CHSRIhandlerITF {
public:
  static const TypeString_t _qryType; //"IAPQuerySRI"

  explicit IAPQuerySRI(IAPQueryId q_id) : IAPQueryAC(q_id)
  {
    mkTaskName();
  }
  virtual ~IAPQuerySRI()
  {
    finalize();
  }

  void configure(const IAPQuerySRI_CFG & use_cfg, Logger * use_log = NULL)
  {
    _cfg = use_cfg;
    if (use_log)
      _logger = use_log;
  }
  // -------------------------------------------------------
  // -- IAPQueryAC interface methods
  // -------------------------------------------------------
  virtual const TypeString_t & taskType(void) const /*throw()*/ { return _qryType; }
  //Starts query execution. May be called only at qryIdle state. 
  //In case of success switches FSM to qryStarted state,
  //otherwise to qryStopping or qryDone state.
  virtual ProcResult_e  start(void) /*throw()*/;

protected:
  //Blocks until all used resources are released.
  //Called only at qryStoppping state.
  virtual ProcResult_e  finalize(void) /*throw()*/;

  friend class smsc::inman::inap::chsri::MapCHSRIDlg;
  // -------------------------------------------------------
  // -- CHSRIhandlerITF implementation:
  // -------------------------------------------------------
  virtual void onMapResult(CHSendRoutingInfoRes & res);
  //if err_code != 0, no result has been got from MAP service,
  //NOTE: dialog user may destroy reporting MAP dialog from this callback in two ways:
  //   1) by calling MapDialogAC::releaseThis() if dialog is allocated on heap
  //   2) by calling ObjFinalizerIface::finalizeObj() and then MapDialogAC destructor
  virtual ObjAllcStatus_e onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash err_code = 0);
  //
  virtual void Awake(void) { notify(); }

private:
  IAPQuerySRI_CFG _cfg;
  smsc::util::OptionalObj_T<smsc::inman::inap::chsri::MapCHSRIDlg>  _mapDlg;
};

/* ************************************************************************** *
 * 
 * ************************************************************************** */
class IAPQueriesPoolSRI : public IAPQueriesPoolIface {
protected:
  //queries are reusable !!
  typedef smsc::core::buffers::IntrusivePoolOfIfaceUnique_T<
    IAPQueryAC, IAPQuerySRI, IAPQueryId
  > QueriesPool;

  QueriesPool       mObjPool; //objects are reusable
  IAPQuerySRI_CFG   mCfg;
  Logger *          mLogger;

public:
  IAPQueriesPoolSRI()
  { }
  virtual ~IAPQueriesPoolSRI()
  { }

  //asigns parameters to use while configuring newly allocated IAPQueries
  void init(const IAPQuerySRI_CFG & use_cfg, Logger * use_log)
  {
    mCfg = use_cfg; mLogger = use_log;
  }

  // -------------------------------------------------------
  // -- IAPQueriesPoolIface interface methods
  // -------------------------------------------------------
  virtual IAPQueryRef   allcQuery(void) /*throw()*/;
  virtual IAPQueryRef   atQuery(IAPQueryId qry_id) /*throw()*/ { return mObjPool.atObj(qry_id); }
  virtual void          reserveObj(IAPQueryId num_obj) /*throw()*/ { return mObjPool.reserve(num_obj); }
};

} //sri
} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPQUERY_HLR_SRI_HPP */

