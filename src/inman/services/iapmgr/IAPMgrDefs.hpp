/* ************************************************************************** *
 * IAPManager (abonent policies/providers manager) types, interfaces and
 * helpers definitions.
 * ************************************************************************** */
#ifndef __INMAN_IAPMANAGER_DEFS_HPP__
#ident "@(#)$Id$"
#define __INMAN_IAPMANAGER_DEFS_HPP__

#include "inman/common/RPCList.hpp"
#include "inman/common/ObjRegistryT.hpp"
#include "inman/abprov/IAProvider.hpp"
#include "inman/services/ICSrvIDs.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {

using smsc::inman::ICSUId;
using smsc::inman::common::RPCList;
using smsc::util::STRKeyRegistry_T;
using smsc::inman::iaprvd::IAProviderITF;
using smsc::inman::iaprvd::IAPQueryProcessorITF;

typedef std::map<std::string, const INScfCFG *> INScfsMAP;
typedef STRKeyRegistry_T<INScfCFG> SCFRegistry;

struct IAProviderInfo {
  ICSUId          _icsUId;
  IAProviderITF * _iface;

  IAProviderInfo(ICSUId  use_uId = ICSIdent::icsIdUnknown,
                 IAProviderITF * use_iface = NULL)
    : _icsUId(use_uId), _iface(use_iface)
  { }
};
typedef STRKeyRegistry_T<IAProviderInfo> IAPrvdsRegistry;

class AbonentPolicy {
protected:
    IAProviderITF * _provd;

public:
    std::string     ident;
    std::string     prvdNm;
    INScfsMAP       scfMap;


    AbonentPolicy(const char * use_id, const char * use_prvd = NULL)
        : ident(use_id), _provd(0)
    {
        if (use_prvd)
            prvdNm = use_prvd;
    }

    bool Init(const IAPrvdsRegistry & prvd_reg)
    {
        if (!prvdNm.empty()) {
          IAProviderInfo * pInfo = prvd_reg.find(prvdNm);
          if (pInfo)
            _provd = pInfo->_iface;
          return _provd ? true : false;
        }
        return true;
    }

    const char * Ident(void) const { return ident.c_str(); }
    const INScfsMAP & ScfMap(void) const { return scfMap; }

    const INScfCFG * getSCFparms(const TonNpiAddress* scf) const
    {
        INScfsMAP::const_iterator it = scfMap.find(scf->toString());
        return (it != scfMap.end()) ? it->second : NULL;
    }

    IAProviderITF::Ability_e getIAPAbilities(void) const
    {
        return _provd ? _provd->ability() : IAProviderITF::abNone;
    }

    IAProviderITF * getIAProvider(void) const
    {
        return  _provd;
    }
};

class PoliciesCfgReg : public STRKeyRegistry_T<AbonentPolicy> {
public:
    bool Init(const IAPrvdsRegistry & prvd_reg)
    {
        bool failed = false;
        for (TRegistry::iterator sit = registry.begin(); sit != registry.end(); ++sit)
            failed |= !sit->second->Init(prvd_reg);
        return !failed;
    }
};

class IAPManagerITF {
protected:
    virtual ~IAPManagerITF() //forbid interface destruction
    { }

public:
    virtual const AbonentPolicy * getPolicy(const std::string & nm_pol) const = 0;
};

struct IAPManagerCFG {
    std::auto_ptr<SCFRegistry>    scfReg;   //registry of INScfCFG
    std::auto_ptr<IAPrvdsRegistry> prvdReg;  //registry of IAProviderITF
    PoliciesCfgReg  polReg;                 //registry of IAPolicies configurations
    ICSIdsSet       deps;

    IAPManagerCFG()
    { }
};

} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_IAPMANAGER_DEFS_HPP__ */

