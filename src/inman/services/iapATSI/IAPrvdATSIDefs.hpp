/* ************************************************************************** *
 * IAProvider utilizing MAP service ATSI.
 * INMan configurable services facility interfaces and classes definition. 
 * ************************************************************************** */
#ifndef __INMAN_IAPRVD_ATSI_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPRVD_ATSI_DEFS_HPP

#include "inman/inap/TCUsrDefs.hpp"
#include "inman/services/ICSrvIDs.hpp"

namespace smsc  {
namespace inman {
namespace iaprvd {
namespace atih {

using smsc::inman::inap::TCAPUsr_CFG;

struct IAProviderATSI_XCFG {
  uint16_t        _maxThreads;
  TCAPUsr_CFG     _atsiCfg;
  ICSIdsSet       _deps;
};


} //atih
} //iaprvd
} //inman
} //smsc
#endif /* __INMAN_IAPRVD_ATSI_DEFS_HPP */

