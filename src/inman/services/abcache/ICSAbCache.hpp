#pragma ident "$Id$"
/* ************************************************************************** *
 * Abonents cache service (contract and gsmSCF parameters).
 * ************************************************************************** */
#ifndef __INMAN_ICS_ABNT_CACHE__
#define __INMAN_ICS_ABNT_CACHE__

#include "inman/incache/solid/AbCacheMT.hpp"
//using smsc::inman::cache::AbonentCacheMTR;
//using smsc::inman::cache::AbonentCacheITF;
//using smsc::inman::cache::AbonentCacheCFG;

#include "inman/services/ICSrvDefs.hpp"
using smsc::inman::ICServiceAC_T;
using smsc::inman::ICSIdent;

namespace smsc {
namespace inman {
namespace cache {

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
    inline Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void)
    {
        cache.reset(new AbonentCacheMTR(*_cfg.get(), logger));
        return ICServiceAC::icsRcOk;
    }
    //Starts service verifying that all dependent services are started
    inline RCode _icsStart(void)
    {
        return cache->Start() ? ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
    }
    //Stops service
    inline void  _icsStop(bool do_wait = false)
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
    inline void * Interface(void) const { return cache.get(); }
    // -------------------------------------
    // AbonentCacheITF interface methods:
    // -------------------------------------
    inline AbonentContractInfo::ContractType
            getAbonentInfo(const AbonentId & ab_number,
                           AbonentRecord * ab_rec = NULL, uint32_t exp_timeout = 0)
    {
        return cache->getAbonentInfo(ab_number, ab_rec, exp_timeout);
    }
    inline void setAbonentInfo(const AbonentId & ab_number, const AbonentRecord & ab_rec)
    {
        cache->setAbonentInfo(ab_number, ab_rec);
    }
};

} //cache
} //inman
} //smsc
#endif /* __INMAN_ICS_ABNT_CACHE__ */

