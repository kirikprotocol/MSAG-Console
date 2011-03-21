/* ************************************************************************** *
 * HLR(ATSI) Abonent Provider: implements functionality for quering the HLR 
 * for various abonent's Camel Subscription Information using
 * ANY_TIME_SUBSCRIPTION_INTERROGATION service.
 * Determines abonent contract and various gsmSCF parameters (address & 
 * serviceKey) in case of prepaid type of contract.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPQUERY_HLR_ATSI_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAPQUERY_HLR_ATSI_HPP

#include "inman/common/OptionalObjT.hpp"
#include "inman/abprov/facility/IAPThrFacility.hpp"
#include "inman/inap/map_atih/DlgMapATSI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace atih { // (A)ny (T)ime (I)nformation (H)andling

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

using smsc::inman::iaprvd::IAPQueryAC;
using smsc::inman::iaprvd::IAPQueryManagerITF;

using smsc::inman::inap::ObjFinalizerIface;
using smsc::inman::inap::ObjAllcStatus_e;

using smsc::inman::inap::TCSessionMA;
using smsc::inman::inap::atih::MapATSIDlg;
using smsc::inman::inap::atih::ATSIhandlerITF;

using smsc::inman::comp::UnifiedCSI;
using smsc::inman::comp::atih::RequestedSubscription;
using smsc::inman::comp::atih::ATSIRes;


class IAPQueryATSI_CFG  : public RequestedSubscription {
public:
  unsigned        mapTimeout;
  TCSessionMA *   mapSess;

  IAPQueryATSI_CFG()
    : RequestedSubscription(UnifiedCSI::csi_O_BC)
    , mapTimeout(20), mapSess(NULL)
  {
    setRequestedODB();
  }
  ~IAPQueryATSI_CFG()
  { }
};

class IAPQueryATSI : public IAPQueryAC, ATSIhandlerITF {
public:
  static const TypeString_t _qryType; //"IAPQueryATSI"

  IAPQueryATSI(unsigned q_id, IAPQueryManagerITF * owner, 
             Logger * use_log, const IAPQueryATSI_CFG & use_cfg);
  // ****************************************
  //-- IAPQueryAC implementation:
  // ****************************************
  int Execute(void);
  void stop(void);
  const TypeString_t & taskType(void) const { return _qryType; }

protected:
  friend class smsc::inman::inap::atih::MapATSIDlg;
  // ****************************************
  //-- ATSIhandlerITF implementation:
  // ****************************************
  virtual void onATSIResult(ATSIRes & arg);
  //if err_code != 0, no result has been got from MAP service,
  //NOTE: dialog user may destroy reporting MAP dialog from this callback in two ways:
  //   1) by calling MapDialogAC::releaseThis() if dialog is allocated on heap
  //   2) by calling ObjFinalizerIface::finalizeObj() and then MapDialogAC destructor
  virtual ObjAllcStatus_e onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash ercode = 0);
  //
  virtual void Awake(void) { _mutex.notify(); }


  IAPQueryATSI_CFG  _cfg;
  smsc::util::OptionalObj_T<MapATSIDlg> _mapDlg;
};

class IAPQueryATSIFactory : public IAPQueryFactoryITF {
private:
  IAPQueryATSI_CFG  _cfg;
  Logger *        logger;

public:
  IAPQueryATSIFactory(const IAPQueryATSI_CFG &in_cfg, unsigned timeout_secs,
                 Logger * uselog = NULL);
  ~IAPQueryATSIFactory()
  { }

  // ****************************************
  //-- IAPQueryFactoryITF implementation:
  // ****************************************
  IAPQueryAC * newQuery(unsigned q_id, IAPQueryManagerITF * owner, Logger * use_log);
};

} //atih
} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPQUERY_HLR_ATSI_HPP */

