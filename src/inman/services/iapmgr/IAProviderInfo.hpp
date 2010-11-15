/* ************************************************************************** *
 * IAPManager: IAProvider data housekeeping helpers.
 * ************************************************************************** */
#ifndef __INMAN_IAPMANAGER_IAPROVIDER_INFO_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPMANAGER_IAPROVIDER_INFO_HPP

#include "inman/abprov/IAProvider.hpp"
#include "inman/services/ICSrvIDs.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {

using smsc::inman::ICSUId;
using smsc::inman::iaprvd::IAProviderAC;

struct IAProviderInfo {
  static const size_t _maxProviderNameSZ = 64;
  typedef smsc::core::buffers::FixedLengthString<_maxProviderNameSZ> NameString_t;

  NameString_t    _ident;
  ICSUId          _icsUId;
  IAProviderAC *  _iface;

  explicit IAProviderInfo(const char * use_name = NULL,
                          ICSUId ics_uid = ICSIdent::icsIdUnknown)
    : _ident(use_name), _icsUId(ics_uid), _iface(NULL)
  { }
};
typedef IAProviderInfo::NameString_t IAProviderName_t;

} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_IAPMANAGER_IAPROVIDER_INFO_HPP */

