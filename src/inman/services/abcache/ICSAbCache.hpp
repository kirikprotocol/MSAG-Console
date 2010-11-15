/* ************************************************************************** *
 * Abonents cache service (contract and gsmSCF parameters).
 * ************************************************************************** */
#ifndef __INMAN_ICS_ABNT_CACHE__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_ABNT_CACHE__

#include "inman/services/ICSrvDefs.hpp"
#include "inman/incache/solid/AbCacheMT.hpp"

namespace smsc {
namespace inman {
namespace cache {

using smsc::inman::ICSIdent;
using smsc::inman::ICServiceAC_T;

//Abonents cache service
class ICSAbntCache : public ICServiceAC_T<AbonentCacheCFG>,
                     public AbonentCacheITF {
protected:
    mutable Mutex   _sync;
    std::auto_ptr<AbonentCacheCFG> _cfg;
    std::auto_ptr<AbonentCacheMTR> cache;

    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void)
    {
        cache.reset(new AbonentCacheMTR(*_cfg.get(), logger));
        return ICServiceAC::icsRcOk;
    }
    //Starts service verifying that all dependent services are started
    RCode _icsStart(void)
    {
        return cache->Start() ? ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
    }
    //Stops service
    void  _icsStop(bool do_wait = false)
    {
        cache->Stop(do_wait);
    }

public:
    ICSAbntCache(std::auto_ptr<AbonentCacheCFG> & use_cfg,
                    ICServicesHostITF * svc_host, Logger * use_log)
        : ICServiceAC_T<AbonentCacheCFG>(
            ICSIdent::icsIdAbntCache, svc_host, use_cfg, use_log)
        , _cfg(use_cfg.release())
    {
        _icsState = ICServiceAC::icsStConfig;
    }
    ~ICSAbntCache()
    {
        ICSStop(true);
    }

    //Returns AbonentCacheITF
    void * Interface(void) const { return cache.get(); }
    // -------------------------------------
    // AbonentCacheITF interface methods:
    // -------------------------------------
    virtual AbonentContract_e
      getAbonentInfo(const AbonentId & ab_number,
                     AbonentSubscription * ab_rec = NULL, uint32_t exp_timeout = 0)
    {
        return cache->getAbonentInfo(ab_number, ab_rec, exp_timeout);
    }
    virtual void 
      setAbonentInfo(const AbonentId & ab_number, const AbonentSubscription & ab_rec)
    {
        cache->setAbonentInfo(ab_number, ab_rec);
    }
};

} //cache
} //inman
} //smsc
#endif /* __INMAN_ICS_ABNT_CACHE__ */

