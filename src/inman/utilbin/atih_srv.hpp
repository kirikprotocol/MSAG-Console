/* ************************************************************************** *
 * Simple console application testing MAP Any Time Subscription Interrogation
 * service of HLR.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_ATIH_SERVICE__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ATIH_SERVICE__

#include <map>

#include "inman/common/OptionalObjT.hpp"
#include "inman/comp/map_atih/MapATSIComps.hpp"
#include "inman/inap/map_atih/DlgMapATSI.hpp"
#include "inman/inap/dispatcher.hpp"
#include "inman/tests/AbonentInfo.hpp"
#include "inman/utilbin/MAPUsrCfg.hpp"

namespace smsc  {
namespace inman {

using smsc::inman::test::AbonentInfo;

using smsc::inman::comp::atih::RequestedSubscription;
using smsc::inman::comp::atih::ATSIRes;

using smsc::inman::inap::ObjFinalizerIface;
using smsc::inman::inap::ObjAllcStatus_e;

using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::inap::TCSessionMA;
using smsc::inman::inap::MAPUsr_CFG;

using smsc::inman::inap::atih::ATSIhandlerITF;
using smsc::inman::inap::atih::MapATSIDlg;

class AT_CSIListenerIface {
protected:
  virtual ~AT_CSIListenerIface()
  { }

public:
  virtual void onCSIresult(const AbonentInfo & ab_info) = 0;
  virtual void onCSIabort(const TonNpiAddress & subcr_addr, RCHash ercode) = 0;
};

class ATIInterrogator : ATSIhandlerITF {
public:
  ATIInterrogator(TCSessionMA * pSession, AT_CSIListenerIface * csi_listener,
                  Logger * uselog = NULL);
  ~ATIInterrogator();

  bool isActive(void);
  //sets subscriber identity: IMSI or MSISDN addr
  bool interrogate(const RequestedSubscription & req_cfg,
                   const TonNpiAddress & subcr_adr);
  void cancel(void);

protected:
  friend class smsc::inman::inap::atih::MapATSIDlg;
  // ---------------------------------------
  // -- ATSIhandlerITF interface
  // ---------------------------------------
  virtual void onATSIResult(ATSIRes & res);
  //if err_code != 0, no result has been got from MAP service,
  //NOTE: dialog user may destroy reporting MAP dialog from this callback in two ways:
  //   1) by calling MapDialogAC::releaseThis() if dialog is allocated on heap
  //   2) by calling ObjFinalizerIface::finalizeObj() and then MapDialogAC destructor
  virtual ObjAllcStatus_e onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash err_code = 0);
  //
  virtual void Awake(void) { _sync.notify(); }

private:
  mutable EventMonitor  _sync;
  volatile bool         _active;
  TCSessionMA *         tcSesssion;
  smsc::util::OptionalObj_T<MapATSIDlg> _mapDlg;
  AT_CSIListenerIface *  csiHdl;
  RequestedSubscription _reqCfg;
  AbonentInfo           _abnInfo;
  Logger *              logger;

  void rlseMapDialog(void);
};

struct ServiceATIH_CFG {
  MAPUsr_CFG      mapCfg;
  AT_CSIListenerIface * client;
};

class ServiceATIH: AT_CSIListenerIface {
public:
  ServiceATIH(const ServiceATIH_CFG & in_cfg, Logger * uselog = NULL);
  virtual ~ServiceATIH();

  bool start();
  void stop(bool do_wait = false);

  RequestedSubscription & getRequestCfg(void) { return _reqCfg; }
  const RequestedSubscription & getRequestCfg(void) const { return _reqCfg; }

  //sets subscriber identity: MSISDN addr
  bool requestCSI(const TonNpiAddress & subcr_addr);

protected:
  friend class ATIInterrogator;
  // -----------------------------------
  // -- AT_CSIListenerIface interface
  // -----------------------------------
  virtual void onCSIresult(const AbonentInfo & ab_info);
  virtual void onCSIabort(const TonNpiAddress & subcr_addr, RCHash ercode);

private:
  typedef std::map<TonNpiAddress, ATIInterrogator *> IntrgtrMAP;
  typedef std::list<ATIInterrogator *> IntrgtrLIST;

  bool getSession(void);
  ATIInterrogator * newWorker(void);

  mutable Mutex     _sync;
  Logger *          logger;
  const char *      _logId;
  TCSessionMA *     mapSess;
  TCAPDispatcher *  disp;
  volatile bool     running;
  ServiceATIH_CFG   _cfg;
  IntrgtrMAP        workers;
  IntrgtrLIST       pool;
  RequestedSubscription _reqCfg;
};


} //inman
} //smsc

#endif /* __SMSC_INMAN_ATIH_SERVICE__ */

