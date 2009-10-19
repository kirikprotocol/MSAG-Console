#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/abdtcr/ICSAbntDtcrProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderAbntDetector(void)
{
  return new smsc::inman::abdtcr::ICSProdAbntDetector();
}


} //inman
} //smsc

