/* ************************************************************************** *
 * IAPManager (abonent policies/providers manager) types, interfaces and
 * helpers definitions.
 * ************************************************************************** */
#ifndef __INMAN_IAPMANAGER_CFG_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPMANAGER_CFG_HPP

#include "core/buffers/XTree.hpp"

#include "inman/services/iapmgr/SCFRegistry.hpp"
#include "inman/services/iapmgr/IAPrvdsRegistry.hpp"
#include "inman/services/iapmgr/AbonentPolicy.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {

//key is AddressPoolMask_t
typedef smsc::core::buffers::XTree<IAPRule> AddressPoolReg;

class PoliciesCfgReg : public smsc::util::POBJRegistry_T<AbonentPolicyName_t, AbonentPolicy> {
public:
  PoliciesCfgReg() : smsc::util::POBJRegistry_T<AbonentPolicyName_t, AbonentPolicy>()
  { }
  ~PoliciesCfgReg()
  { }

  bool bindProviders(const IAPrvdsRegistry & prvd_reg);
};

struct IAPManagerCFG {
  std::auto_ptr<SCFRegistry>      scfReg;       //registry of INScfCFG
  std::auto_ptr<IAPrvdsRegistry>  prvdReg;      //registry of IAProviderAC
  PoliciesCfgReg                  policiesReg;  //registry of IAPolicies configurations
  AddressPoolReg                  poolsReg;     //Address polls registry
  ICSIdsSet                       deps;         //IAPManager service dependencies

  IAPManagerCFG()
  { }
  ~IAPManagerCFG()
  { }

  //constructs address pools registry from policies registry
  int initPoolsRegistry(void); 
};

} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_IAPMANAGER_CFG_HPP */

