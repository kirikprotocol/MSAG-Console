/* ************************************************************************** *
 * IAProvider utilizing MAP service CH-SRI.
 INMan configurable services facility interfaces and classes definition. 
 * ************************************************************************** */
#ifndef __INMAN_IAPRVD_SRI_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPRVD_SRI_DEFS_HPP

#include "inman/inap/TCUsrDefs.hpp"
#include "inman/services/ICSrvIDs.hpp"

namespace smsc  {
namespace inman {
namespace iaprvd {
namespace sri {

using smsc::inman::inap::TCAPUsr_CFG;

struct IAProviderSRI_XCFG {
  uint16_t        _maxThreads;
  TCAPUsr_CFG     _sriCfg;
  ICSIdsSet       _deps;

  IAProviderSRI_XCFG() : _maxThreads(0)
  { }
};

} //sri
} //iaprvd
} //inman
} //smsc
#endif /* __INMAN_IAPRVD_SRI_DEFS_HPP */

