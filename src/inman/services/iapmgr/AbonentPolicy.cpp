#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapmgr/AbonentPolicy.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {
/* ************************************************************************** *
 * class ISDNAddressMask implementation:
 * ************************************************************************** */
const char * ISDNAddressMask::_numberingFmt = "%15[0-9\?]%2s";

bool ISDNAddressMask::fromText(const char * in_text, char * out_str)
{
  char  trash[4];
  int   scanned = sscanf(in_text, _numberingFmt, out_str, trash);
  return (scanned == 1) && (strlen(out_str) >= 5);
}

/* ************************************************************************** *
 * class AbonentPolicy implementation:
 * ************************************************************************** */

//Returns true if at least one AbonentProvider is initialized
bool AbonentPolicy::bindProviders(const IAPrvdsRegistry & prvd_reg)
{
  bool rval = false;
  for (IAPrvdsLIST::iterator it = _prvdList.begin(); it != _prvdList.end(); ++it) {
    const IAProviderInfo * pInfo = prvd_reg.find(it->_ident);
    if (pInfo) {
      it->_icsUId = pInfo->_icsUId;
      it->_iface = pInfo->_iface;
      rval = true;
    }
  }
  return rval;
}

//Returns frist initialized IAProvider in prioritized list that supports requested option.
const IAProviderInfo * 
  AbonentPolicy::hasAbility(IAPAbility::Option_e op_val) const
{
  for (IAPrvdsLIST::const_iterator cit = _prvdList.begin(); cit != _prvdList.end(); ++cit) {
    if (cit->_iface && cit->_iface->getAbility().hasOption(op_val)) {
      return cit.operator->();
    }
  }
  return NULL;
}

//Returns next initialized IAProvider following the given one in prioritized list.
const IAProviderInfo *
  AbonentPolicy::getIAProvider(IAPType_e prev_prvd/* = IAPType::iapUnknown*/) const
{
  const IAProviderInfo * iapInfo = NULL;
  if (prev_prvd != IAPProperty::iapUnknown) {
    //search specified IAProvider first
    for (IAPrvdsLIST::const_iterator cit = _prvdList.begin(); cit != _prvdList.end(); ++cit) {
      if (cit->_iface && cit->_iface->getProperty().isEqual(prev_prvd)) {
        IAPrvdsLIST::const_iterator tmpIt = cit;
        return ((++tmpIt) != _prvdList.end()) ? tmpIt.operator->() : NULL;
      }
    }
  }
  if (!iapInfo) { //search first initialized IAProvider
    for (IAPrvdsLIST::const_iterator cit = _prvdList.begin(); cit != _prvdList.end(); ++cit) {
      if (cit->_iface)
        return cit.operator->();
    }
  }
  return NULL; //all are uninitialized
}


} //iapmgr
} //inman
} //smsc

