#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/tcapdsp/ICSTCDspProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderTCAPDispatcher(void)
{
  return new smsc::inman::inap::ICSProdTCAPDispatcher();
}

} //inman
} //smsc

