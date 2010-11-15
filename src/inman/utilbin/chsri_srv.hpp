/* ************************************************************************** *
 * Simple console application testing MAP Send Routing Info (Call Handling)
 * service of HLR.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_CHSRI_SERVICE__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_CHSRI_SERVICE__

#include <map>

#include "inman/comp/map_chsri/MapCHSRIComps.hpp"
#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"
#include "inman/inap/dispatcher.hpp"
#include "inman/tests/AbonentInfo.hpp"
#include "inman/utilbin/MAPUsrCfg.hpp"

namespace smsc  {
namespace inman {

using smsc::util::IMSIString;
using smsc::inman::test::AbonentInfo;

using smsc::inman::comp::chsri::CHSendRoutingInfoRes;

using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::inap::TCSessionMA;
using smsc::inman::inap::MAPUsr_CFG;

using smsc::inman::inap::chsri::CHSRIhandlerITF;
using smsc::inman::inap::chsri::MapCHSRIDlg;

class SRI_CSIListenerIface {
protected:
  virtual ~SRI_CSIListenerIface()
  { }

public:
  virtual void onCSIresult(const AbonentInfo & ab_info) = 0;
  virtual void onCSIabort(const TonNpiAddress & subcr_addr, RCHash ercode) = 0;
};

class SRIInterrogator: CHSRIhandlerITF {
public:
  SRIInterrogator(TCSessionMA * pSession, SRI_CSIListenerIface * csi_listener,
                  Logger * uselog = NULL);
  ~SRIInterrogator();

  bool isActive(void);
  //sets subscriber identity: MSISDN addr
  bool interrogate(const TonNpiAddress & subcr_addr);
  void cancel(void);

protected:
  friend class smsc::inman::inap::chsri::MapCHSRIDlg;
  // ------------------------------------
  // -- CHSRIhandlerITF implementation:
  // ------------------------------------
  virtual void onMapResult(CHSendRoutingInfoRes & res);
  //dialog finalization/error handling:
  //if ercode != 0, no result has been got from MAP service,
  virtual void onEndMapDlg(RCHash ercode = 0);
  //
  virtual void Awake(void) { _sync.notify(); }

private:
  mutable EventMonitor    _sync;
  volatile bool           _active;
  TCSessionMA *           tcSesssion;
  MapCHSRIDlg *           sriDlg;
  SRI_CSIListenerIface *  csiHdl;
  AbonentInfo             _abnInfo;
  Logger *                logger;

  void rlseSRIDialog(void);
};


struct ServiceCHSRI_CFG {
  MAPUsr_CFG      mapCfg;
  SRI_CSIListenerIface * client;
};

class ServiceCHSRI: SRI_CSIListenerIface {
public:
  explicit ServiceCHSRI(const ServiceCHSRI_CFG & in_cfg, Logger * uselog = NULL);
  ~ServiceCHSRI();

  bool start(void);
  void stop(bool do_wait = false);

  //sets subscriber identity: MSISDN addr
  bool requestCSI(const TonNpiAddress & subcr_addr);

protected:
  friend class SRIInterrogator;
  // -------------------------------------
  // -- SRI_CSIListenerIface interface
  // -------------------------------------
  virtual void onCSIresult(const AbonentInfo & ab_info);
  virtual void onCSIabort(const TonNpiAddress & subcr_addr, RCHash ercode);

private:
  typedef std::map<TonNpiAddress, SRIInterrogator *> IntrgtrMAP;
  typedef std::list<SRIInterrogator *> IntrgtrLIST;

  mutable Mutex     _sync;
  Logger*           logger;
  const char *      _logId;
  TCSessionMA *     mapSess;
  TCAPDispatcher *  disp;
  volatile bool     running;
  ServiceCHSRI_CFG  _cfg;
  IntrgtrMAP        workers;
  IntrgtrLIST       pool;

  bool getSession(void);
  SRIInterrogator * newWorker(void);
};


} //inman
} //smsc

#endif /* __SMSC_INMAN_CHSRI_SERVICE__ */

