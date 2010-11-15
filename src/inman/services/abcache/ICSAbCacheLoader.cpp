#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abcache/ICSAbCacheProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderAbCache(void)
{
  return new smsc::inman::cache::ICSProdAbCache();
}

} //inman
} //smsc

