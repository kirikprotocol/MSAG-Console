#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/tmwatch/ICSTMWProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderTMWatcher(void)
{
  return new ICSProdTMWatcher();
}

} //inman
} //smsc

