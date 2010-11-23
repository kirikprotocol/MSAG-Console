/* ************************************************************************** *
 * IAPManager: abonent policies types, interfaces and helpers definitions.
 * ************************************************************************** */
#ifndef __INMAN_IAPMANAGER_ABPOLICY_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPMANAGER_ABPOLICY_HPP__

#include <list>

#include "inman/services/iapmgr/SCFRegistry.hpp"
#include "inman/services/iapmgr/IAPrvdsRegistry.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {

class ISDNAddressMask : public smsc::core::buffers::FixedLengthString<MAPConst::MAX_ISDN_AddressValueLength> {
public:
  //static const NumberingParms _numberingParms(5, 15, "0-9");
  static const char * _numberingFmt;// = "%15[0-9\?]%2s";

  static bool fromText(const char * in_text, char * out_str);

  ISDNAddressMask()
    : smsc::core::buffers::FixedLengthString<MAPConst::MAX_ISDN_AddressValueLength>()
  { }
  ISDNAddressMask(const char * use_val)
    : smsc::core::buffers::FixedLengthString<MAPConst::MAX_ISDN_AddressValueLength>(use_val)
  { }

  bool fromText(const char * in_text) { return fromText(in_text, str); }
};

struct AddressPool {
  static const size_t _maxPoolNameSZ = 64;
  typedef smsc::core::buffers::FixedLengthString<_maxPoolNameSZ+1>  PoolName_t;

  PoolName_t      _name;
  ISDNAddressMask _mask;

  explicit AddressPool(const char * use_name = NULL)
    : _name(use_name)
  { }

  bool operator< (const AddressPool & cmp_obj) const
  {
    return _name < cmp_obj._name;
  }
};
typedef AddressPool::PoolName_t AddressPoolName_t;

typedef std::list<std::string>  CStrList;
typedef std::set<AddressPool>   AddressPoolsSet;
typedef std::pair<IAProviderInfo, IAProviderInfo> IAPrvdsPrio;


struct AbonentPolicy {
  static const size_t _maxPolicyNameSZ = 64;
  typedef smsc::core::buffers::FixedLengthString<_maxPolicyNameSZ+1> NameString_t;

  enum IAPrvdPrio_e  {
    iapNone = 0, iapPrimary, iapSecondary
  };

  NameString_t      _ident;
  IAPrvdsPrio       _prvdPrio; //prioritized pair of allowed IAProviders
  SCFParmsMap       _scfMap;
  AddressPoolsSet   _poolsSet;

  explicit AbonentPolicy(const char * use_id = NULL) : _ident(use_id)
  { }
  AbonentPolicy(const char * use_id, const CStrList & nm_prvds);
  //
  ~AbonentPolicy()
  { }

  //Returns true if at least one AbonentProvider is initialized
  bool bindProviders(const IAPrvdsRegistry & prvd_reg);

  const char * getIdent(void) const { return _ident.c_str(); }
  const SCFParmsMap & getScfMap(void) const { return _scfMap; }
  //
  const INScfCFG * getSCFparms(const TonNpiAddress & scf_adr) const
  {
    SCFParmsMap::const_iterator it = _scfMap.find(scf_adr);
    return (it != _scfMap.end()) ? it->second : NULL;
  }

  //Returns initialized IAProvider with given priority
  const IAProviderInfo * getIAProvider(IAPrvdPrio_e prvd_prio) const;
};
typedef AbonentPolicy::NameString_t  AbonentPolicyName_t;
typedef AbonentPolicy::IAPrvdPrio_e  IAPPrio_e;


typedef smsc::core::buffers::FixedLengthString <
  AbonentPolicy::_maxPolicyNameSZ + AddressPool::_maxPoolNameSZ + 2
> AddressPoolFullName_t;

struct IAPRule { //Abonent policy rule
  AddressPoolName_t     _nmPool;
  const AbonentPolicy * _iaPolicy;

  explicit IAPRule(const char * nm_pool = NULL, const AbonentPolicy * ia_policy = NULL)
    : _nmPool(nm_pool), _iaPolicy(ia_policy)
  { }

  AddressPoolFullName_t toString(void) const
  {
    AddressPoolFullName_t rstr(_iaPolicy ? _iaPolicy->getIdent() : "<unknown>");
    rstr += '.';
    rstr += _nmPool;
    return rstr;
  }
};

} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_IAPMANAGER_ABPOLICY_HPP__ */

