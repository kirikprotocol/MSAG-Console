/* ************************************************************************* *
 * IAProvider service utilizing MAP ATSI service.
 * ************************************************************************* */
#ifndef __INMAN_ICS_IAPRVD_ATSI_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_IAPRVD_ATSI_HPP

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/iapATSI/IAPrvdATSIDefs.hpp"
#include "inman/services/iapATSI/IAPQueryATSI.hpp"
#include "inman/abprov/IAProvider.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace atih {

using smsc::inman::ICServiceAC_T;
using smsc::inman::iaprvd::IAPQueryProcessorITF;
using smsc::inman::iaprvd::IAProviderAC;


struct IAPrvdATSI_CFG {
  unsigned        init_threads;
  TCAPUsr_CFG     atsiCfg;
  IAPQueryATSI_CFG qryCfg;

  IAPrvdATSI_CFG() : init_threads(0)
  { }
  IAPrvdATSI_CFG(const TCAPUsr_CFG & use_cfg)
    : init_threads(0), atsiCfg(use_cfg)
  {
    qryCfg.mapTimeout = atsiCfg.rosTimeout;
  }
};


class ICSIAPrvdATSI : public ICServiceAC_T<IAProviderATSI_XCFG>,
                      public IAProviderAC {
private:
  mutable Mutex   _sync;
  const char *    _logId; //logging prefix

  IAPrvdATSI_CFG  _cfg;
  IAPFacilityCFG  _fcltCfg;

  std::auto_ptr<IAPQueryATSIFactory>  _qryPlant;
  std::auto_ptr<IAPQueryFacility>     _prvd;

protected:
  // ---------------------------------
  // -- ICServiceAC interface methods
  // --------------------------------- 
  Mutex & _icsSync(void) const { return _sync; }
  //Initializes service verifying that all dependent services are inited
  RCode _icsInit(void);
  //Starts service verifying that all dependent services are started
  RCode _icsStart(void);
  //Stops service
  void  _icsStop(bool do_wait = false)
  {
    if (_prvd.get())
      _prvd->Stop(do_wait);
  }

public:
  ICSIAPrvdATSI(std::auto_ptr<IAProviderATSI_XCFG> & use_cfg,
                         const ICServicesHostITF * svc_host, Logger * use_log = NULL)
    : ICServiceAC_T<IAProviderATSI_XCFG>(ICSIdent::icsIAPrvdATSI, svc_host, use_cfg, use_log)
    , IAProviderAC(IAPProperty::iapATSI)
    , _logId("iapATSI"), _cfg(use_cfg->atsiCfg)
  {
    _fcltCfg.init_threads = _cfg.init_threads;
    _fcltCfg.max_queries = _cfg.atsiCfg.maxDlgId;
    _fcltCfg.qryMultiRun = true; //MapATSI dialogs are reused !!!
    _fcltCfg.qryPlant = NULL;    //will be inited by _icsInit() later
    //
    _icsDeps = use_cfg->deps;
    _icsState = ICServiceAC::icsStConfig;
  }
  ~ICSIAPrvdATSI()
  {
    ICSStop(true);
  }

  //Returns IAProviderAC
  void * Interface(void) const { return (IAProviderAC*)this; }

  // ----------------------------------
  // -- IAPQueryProcessorITF interface methods
  // ----------------------------------
  //Starts query and binds listener to it.
  //Returns true if query succesfully started, false otherwise
  bool startQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb)
  {
    return _prvd.get() ? _prvd->startQuery(ab_number, pf_cb) : false;
  }
  //Unbinds query listener, cancels query if no listeners remain.
  void cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb)
  {
    if (_prvd.get())
      _prvd->cancelQuery(ab_number, pf_cb);
  }
  void cancelAllQueries(void)
  {
    if (_prvd.get())
      _prvd->cancelAllQueries();
  }

  // ----------------------------------
  // -- IAProviderAC interface methods
  // ----------------------------------
  virtual void  logConfig(Logger * use_log = NULL) const;
};

} //sri
} //iaprvd
} //inman
} //smsc
#endif /* __INMAN_ICS_IAPRVD_ATSI_HPP */

