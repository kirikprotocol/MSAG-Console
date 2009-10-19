#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/ICSrvLoader.hpp"
#include "inman/services/abcache/ICSAbCacheLoader.hpp"

namespace smsc {
namespace inman {

extern "C" ICSProducerAC * loadupICService(void)
{
    return ICSLoaderAbCache();
}

} //inman
} //smsc

