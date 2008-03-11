#ifndef MOD_IDENT_OFF
static const char ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/ICSrvLoader.hpp"
using smsc::inman::ICSProducerAC;

#include "inman/services/abdtcr/ICSAbntDtcrProd.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

//This is the TCAP Dispatcher Service dynamic library entry point
extern "C" ICSProducerAC * loadupICService(void)
{
    return new ICSProdAbntDetector();
}

} //abdtcr
} //inman
} //smsc

