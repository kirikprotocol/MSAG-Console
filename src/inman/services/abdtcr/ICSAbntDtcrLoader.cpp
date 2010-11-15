#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abdtcr/ICSAbntDtcrProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderAbntDetector(void)
{
  return new smsc::inman::abdtcr::ICSProdAbntDetector();
}


} //inman
} //smsc

