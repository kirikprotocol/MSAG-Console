#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/tmwatch/ICSTMWProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderTMWatcher(void)
{
  return new ICSProdTMWatcher();
}

} //inman
} //smsc

