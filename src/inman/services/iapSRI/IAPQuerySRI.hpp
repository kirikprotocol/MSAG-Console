/* ************************************************************************** *
 * HLR(SRI) Abonent Provider: implements functionality for quering the HLR 
 * for abonent Camel Subscription Information via SEND_ROUTING_INFO service,
 * determining abonent contract and gsmSCF parameters (address & serviceKey)
 * in case of prepaid type of contract.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPQUERY_HLR_SRI_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAPQUERY_HLR_SRI_HPP

#include "inman/common/OptionalObjT.hpp"
#include "inman/abprov/facility/IAPThrFacility.hpp"
#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri { // (S)end (R)outing (I)nfo

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

using smsc::inman::iaprvd::IAPQueryAC;
using smsc::inman::iaprvd::IAPQueryManagerITF;

using smsc::inman::inap::ObjFinalizerIface;
using smsc::inman::inap::ObjAllcStatus_e;

using smsc::inman::inap::TCSessionMA;
using smsc::inman::inap::chsri::MapCHSRIDlg;
using smsc::inman::inap::chsri::CHSRIhandlerITF;

using smsc::inman::comp::chsri::CHSendRoutingInfoRes;

struct IAPQuerySRI_CFG {
  unsigned        mapTimeout;
  TCSessionMA *   mapSess;

  IAPQuerySRI_CFG() : mapTimeout(20), mapSess(NULL)
  { }
};

class IAPQuerySRI : public IAPQueryAC, CHSRIhandlerITF {
public:
  static const TypeString_t _qryType; //"IAPQuerySRI"

  IAPQuerySRI(unsigned q_id, IAPQueryManagerITF * owner, 
             Logger * use_log, const IAPQuerySRI_CFG & use_cfg);
  // ****************************************
  //-- IAPQueryAC implementation:
  // ****************************************
  int Execute(void);
  void stop(void);
  const TypeString_t & taskType(void) const { return _qryType; }

protected:
  friend class smsc::inman::inap::chsri::MapCHSRIDlg;
  // ****************************************
  //-- CHSRIhandlerITF implementation:
  // ****************************************
  virtual void onMapResult(CHSendRoutingInfoRes & res);
  //if err_code != 0, no result has been got from MAP service,
  //NOTE: dialog user may destroy reporting MAP dialog from this callback in two ways:
  //   1) by calling MapDialogAC::releaseThis() if dialog is allocated on heap
  //   2) by calling ObjFinalizerIface::finalizeObj() and then MapDialogAC destructor
  virtual ObjAllcStatus_e onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash ercode = 0);
  //
  virtual void Awake(void) { _mutex.notify(); }


  IAPQuerySRI_CFG _cfg;
  smsc::util::OptionalObj_T<MapCHSRIDlg> _mapDlg;
};

class IAPQuerySRIFactory : public IAPQueryFactoryITF {
private:
  IAPQuerySRI_CFG  _cfg;
  Logger *        logger;

public:
  IAPQuerySRIFactory(const IAPQuerySRI_CFG &in_cfg, unsigned timeout_secs,
                 Logger * uselog = NULL);
  ~IAPQuerySRIFactory() { }

  // ****************************************
  //-- IAPQueryFactoryITF implementation:
  // ****************************************
  IAPQueryAC * newQuery(unsigned q_id, IAPQueryManagerITF * owner, Logger * use_log);
};

} //sri
} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPQUERY_HLR_SRI_HPP */

