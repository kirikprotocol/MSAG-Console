#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/scheduler/ICSSchedProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderScheduler(void)
{
  return new ICSProdScheduler();
}

} //inman
} //smsc

