#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/scheduler/ICSSchedProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderScheduler(void)
{
  return new ICSProdScheduler();
}

} //inman
} //smsc

