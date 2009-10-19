#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/ICSrvLoader.hpp"
#include "inman/services/iapSRI/ICSIAPrvdSRILoader.hpp"

namespace smsc {
namespace inman {

//ICService dynamic library entry point for
//    IAProvider utilizing MAP service CH-SRI.
extern "C" ICSProducerAC * loadupICService(void)
{
    return smsc::inman::ICSLoaderIAPrvdSRI();
}

} //inman
} //smsc

