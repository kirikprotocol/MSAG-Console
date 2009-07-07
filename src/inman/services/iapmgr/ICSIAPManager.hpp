/* ************************************************************************** *
 * IAPManager (abonent policies/providers manager) service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_IAP_MANAGER__
#ident "@(#)$Id$"
#define __INMAN_ICS_IAP_MANAGER__

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/iapmgr/IAPMgrDefs.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

//Abonent detector service
class ICSIAPManager : public ICServiceAC_T<IAPManagerCFG>,
                     public IAPManagerITF {
protected:
    mutable Mutex   _sync;
    std::auto_ptr<IAPManagerCFG> _cfg;

    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void)
    {
        //bind policies to IAProvider allocators
        return  _cfg->polReg.Init(*(_cfg->prvdReg.get()), _icsHost) ?
            ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
    }
    //Starts service verifying that all dependent services are started
    RCode _icsStart(void)
    {
        bool failed = false;
        PoliciesCfgReg::const_iterator it = _cfg->polReg.begin();
        for (; it != _cfg->polReg.end() && !failed; ++it)
            failed |= !it->second->startIAProvider();
        return failed ? ICServiceAC::icsRcError : ICServiceAC::icsRcOk;
    }
    //Stops service
    void  _icsStop(bool do_wait = false)
    {
        PoliciesCfgReg::const_iterator it = _cfg->polReg.begin();
        for (; it != _cfg->polReg.end(); ++it)
            it->second->stopIAProvider(do_wait);
        return;
    }

public:
    ICSIAPManager(std::auto_ptr<IAPManagerCFG> & use_cfg,
                    ICServicesHostITF * svc_host, Logger * use_log)
        : ICServiceAC_T<IAPManagerCFG>(ICSIdent::icsIdIAPManager,
                                       svc_host, use_cfg, use_log)
        , _cfg(use_cfg.release())
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
    const AbonentPolicy * getPolicy(const std::string & nm_pol) const
    {
        MutexGuard grd(_sync);
        return _cfg->polReg.find(nm_pol);
    }
};

} //iapmgr
} //inman
} //smsc
#endif /* __INMAN_ICS_IAP_MANAGER__ */

