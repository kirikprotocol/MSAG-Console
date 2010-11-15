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

using smsc::inman::inap::atih::MapATSIDlg;
using smsc::inman::inap::atih::ATSIhandlerITF;

using smsc::inman::comp::UnifiedCSI;
using smsc::inman::comp::atih::RequestedSubscription;


class IAPQueryATSI_CFG  : public RequestedSubscription {
public:
  unsigned        mapTimeout;
  TCSessionMA *   mapSess;

  IAPQueryATSI_CFG()
    : RequestedSubscription(UnifiedCSI::csi_MO_SM)
    , mapTimeout(20), mapSess(NULL)
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
  //dialog finalization/error handling:
  //if ercode != 0, dialog is aborted by reason = errcode
  virtual void onEndATSI(RCHash ercode = 0);
  //
  virtual void Awake(void) { _mutex.notify(); }


  IAPQueryATSI_CFG  _cfg;
  MapATSIDlg *      _mapDlg;
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

