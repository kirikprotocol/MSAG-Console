#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/ICSrvLoader.hpp"
#include "inman/services/iapATSI/ICSIAPrvdATSILoader.hpp"

namespace smsc {
namespace inman {

//ICService dynamic library entry point for
//    IAProvider utilizing MAP service CH-SRI.
extern "C" ICSProducerAC * loadupICService(void)
{
    return smsc::inman::ICSLoaderIAPrvdATSI();
}

} //inman
} //smsc

