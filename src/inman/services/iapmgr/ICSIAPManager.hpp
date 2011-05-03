/* ************************************************************************** *
 * IAPManager (abonent policies/providers manager) service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_IAP_MANAGER__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_IAP_MANAGER__

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/iapmgr/IAPMgrCfg.hpp"
#include "inman/services/iapmgr/IAPMgrDefs.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

//Abonent Providers control service
class ICSIAPManager : public ICServiceAC_T<IAPManagerCFG>,
                     public IAPManagerITF {
protected:
    mutable Mutex   _sync;
    const char *    _logId;     //logging prefix
    std::auto_ptr<IAPManagerCFG> _cfg;

    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void);
    //Starts service verifying that all dependent services are started
    RCode _icsStart(void)
    {
        return ICServiceAC::icsRcOk;
    }
    //Stops service
    void  _icsStop(bool do_wait = false) { return; }

public:
    ICSIAPManager(std::auto_ptr<IAPManagerCFG> & use_cfg,
                    ICServicesHostITF * svc_host, Logger * use_log)
        : ICServiceAC_T<IAPManagerCFG>(ICSIdent::icsIdIAPManager,
                                       svc_host, use_cfg, use_log)
        , _logId("iapMgr"), _cfg(use_cfg.release())
    {
        _icsDeps = _cfg->deps;
        _icsState = ICServiceAC::icsStConfig;
    }
    ~ICSIAPManager()
    {
        ICSStop(true);
    }

    //Returns IAPManagerITF
    void * Interface(void) const { return (IAPManagerITF*)this; }

    // -------------------------------------
    // IAPManagerITF interface methods:
    // -------------------------------------
    virtual const AbonentPolicy *
      getPolicy(const AbonentPolicyName_t & nm_pol) const
    {
        MutexGuard grd(_sync);
        return _cfg->policiesReg.find(nm_pol);
    }

    virtual const IAPRule *
      getPolicy(const TonNpiAddress & subscr_addr) const
    {
        MutexGuard grd(_sync);
        return _cfg->poolsReg.Find(subscr_addr.getSignals());
    }
};

} //iapmgr
} //inman
} //smsc
#endif /* __INMAN_ICS_IAP_MANAGER__ */

