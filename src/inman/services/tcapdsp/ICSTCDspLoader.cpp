#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/tcapdsp/ICSTCDspProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderTCAPDispatcher(void)
{
  return new smsc::inman::inap::ICSProdTCAPDispatcher();
}

} //inman
} //smsc

