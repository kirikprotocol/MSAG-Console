#pragma ident "$Id$"
/* ************************************************************************** *
 * IAPManager (abonent policies/providers manager) types, interfaces and
 * helpers definitions.
 * ************************************************************************** */
#ifndef __INMAN_IAPMANAGER_DEFS_HPP__
#define __INMAN_IAPMANAGER_DEFS_HPP__

#include "inman/common/RPCList.hpp"
using smsc::inman::common::RPCList;

#include "inman/common/ObjRegistryT.hpp"
using smsc::util::STRKeyRegistry_T;

#include "inman/abprov/IAProvider.hpp"
using smsc::inman::iaprvd::IAProviderCreatorITF;
using smsc::inman::iaprvd::IAProviderITF;
using smsc::inman::iaprvd::IAProvider;

namespace smsc  {
namespace inman {
namespace iapmgr {

typedef std::map<std::string, const INScfCFG *> INScfsMAP;
typedef STRKeyRegistry_T<INScfCFG> SCFRegistry;
typedef STRKeyRegistry_T<IAProviderCreatorITF> IAPrvdsRegistry;

class AbonentPolicy {
public:
    std::string     ident;
    std::string     prvdNm;
    INScfsMAP       scfMap;
    IAProviderCreatorITF * provAllc;
    const ICServicesHostITF * svcHost;

    AbonentPolicy(const char * use_id, const char * use_prvd = NULL)
        : ident(use_id), provAllc(0)
    {
        if (use_prvd)
            prvdNm = use_prvd;
    }

    bool Init(const IAPrvdsRegistry & prvd_reg, const ICServicesHostITF * use_host)
    {
        svcHost = use_host;
        if (!prvdNm.empty()) {
            provAllc = prvd_reg.find(prvdNm);
            return provAllc ? true : false;
        }
        return true;
    }

    inline const char * Ident(void) const { return ident.c_str(); }
    inline const INScfsMAP & ScfMap(void) const { return scfMap; }

    bool useSS7(void) const
    {
        IAProvider::Type provType = !provAllc ? 
                        IAProvider::iapCACHE : provAllc->type();

        return (!scfMap.empty()
                || (provType == IAProvider::iapIN)
                || (provType == IAProvider::iapHLR));
    }

    const INScfCFG * getSCFparms(const TonNpiAddress* scf) const
    {
        INScfsMAP::const_iterator it = scfMap.find(scf->toString());
        return (it != scfMap.end()) ? it->second : NULL;
    }

    inline IAProvider::Ability getIAPAbilities(void) const
    {
        return provAllc ? provAllc->ability() : IAProvider::abNone;
    }

    inline IAProviderITF * getIAProvider(void) const
    {
        return  provAllc ? provAllc->startProvider(svcHost) : NULL;
    }

    inline bool startIAProvider(void) const
    {
        return !provAllc ? true :
            (provAllc->startProvider(svcHost) != NULL);
    }
    inline void stopIAProvider(bool do_wait = false) const
    {
        if (provAllc)
            provAllc->stopProvider(do_wait);
    }
};

class PoliciesCfgReg : public STRKeyRegistry_T<AbonentPolicy> {
public:
    bool Init(const IAPrvdsRegistry & prvd_reg, const ICServicesHostITF * use_host)
    {
        bool failed = false;
        for (TRegistry::iterator sit = registry.begin(); sit != registry.end(); ++sit)
            failed |= !sit->second->Init(prvd_reg, use_host);
        return !failed;
    }
};

class IAPManagerITF {
public:
    virtual const AbonentPolicy * getPolicy(const std::string & nm_pol) const = 0;
};

struct IAPManagerCFG {
    std::auto_ptr<SCFRegistry>    scfReg;   //registry of INScfCFG
    std::auto_ptr<IAPrvdsRegistry> prvdReg;  //registry of IAProviderCreatorITF
    PoliciesCfgReg  polReg;                 //registry of IAPolicies configurations
    ICSIdsSet       deps;

    IAPManagerCFG()
    { }
};

} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_IAPMANAGER_DEFS_HPP__ */

