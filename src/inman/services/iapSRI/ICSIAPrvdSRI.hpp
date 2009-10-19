/* ************************************************************************* *
 * IAProvider service utilizing MAP CH-SRI service.
 * ************************************************************************* */
#ifndef __INMAN_ICS_IAPRVD_SRI_HPP
#ident "@(#)$Id$"
#define __INMAN_ICS_IAPRVD_SRI_HPP

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/iapSRI/IAPrvdSRIDefs.hpp"
#include "inman/services/iapSRI/IAPQuerySRI.hpp"
#include "inman/abprov/IAProvider.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

using smsc::inman::ICServiceAC_T;
using smsc::inman::iaprvd::IAPQueryProcessorITF;
using smsc::inman::iaprvd::IAProviderITF;


struct IAPrvdSRI_CFG {
  unsigned        init_threads;
  TCAPUsr_CFG     sriCfg;
  IAPQuerySRI_CFG qryCfg;

  IAPrvdSRI_CFG() : init_threads(0)
  { }
  IAPrvdSRI_CFG(const TCAPUsr_CFG & use_cfg)
    : init_threads(0), sriCfg(use_cfg)
  {
    qryCfg.mapTimeout = sriCfg.rosTimeout;
  }
};


class ICSIAPrvdSRI : public ICServiceAC_T<IAProviderSRI_XCFG>,
                      public IAProviderITF {
private:
    mutable Mutex   _sync;
    const char *    _logId; //logging prefix

    IAPrvdSRI_CFG   _cfg;
    IAPFacilityCFG  _fcltCfg;
    std::auto_ptr<IAPQuerySRIFactory> _qryPlant;
    std::auto_ptr<IAPQueryFacility> _prvd;

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
    ICSIAPrvdSRI(std::auto_ptr<IAProviderSRI_XCFG> & use_cfg,
                 const ICServicesHostITF * svc_host, Logger * use_log = NULL)
        : ICServiceAC_T<IAProviderSRI_XCFG>(ICSIdent::icsIAPrvdSRI, svc_host, use_cfg, use_log)
        , _logId("iapSRI"), _cfg(use_cfg->sriCfg)
    {
      _fcltCfg.init_threads = _cfg.init_threads;
      _fcltCfg.max_queries = _cfg.sriCfg.maxDlgId;
      _fcltCfg.qryMultiRun = true; //MapCHSRI dialogs are reused !!!
      _fcltCfg.qryPlant = NULL;    //will be inited by _icsInit() later
      //
      _icsDeps = use_cfg->deps;
      _icsState = ICServiceAC::icsStConfig;
    }
    ~ICSIAPrvdSRI()
    {
      ICSStop(true);
    }

    //Returns IAProviderITF
    void * Interface(void) const { return (IAProviderITF*)this; }

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
    // -- IAProviderITF interface methods
    // ----------------------------------
    IAProviderITF::Type_e     type(void)      const { return IAProviderITF::iapHLR; }
    IAProviderITF::Ability_e  ability(void)   const { return IAProviderITF::abContractSCF; }

    const char *  ident(void) const { return "iapHLR_SRI"; }
    void          logConfig(Logger * use_log = NULL) const;
};

} //sri
} //iaprvd
} //inman
} //smsc
#endif /* __INMAN_ICS_IAPRVD_SRI_HPP */

