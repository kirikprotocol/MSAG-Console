#ifndef MOD_IDENT_OFF
static const char ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/ICSrvLoader.hpp"
using smsc::inman::ICSProducerAC;

#include "inman/services/tcapdsp/ICSTCDspProd.hpp"

namespace smsc {
namespace inman {
namespace inap {

//This is the TCAP Dispatcher Service dynamic library entry point
extern "C" ICSProducerAC * loadupICService(void)
{
    return new ICSProdTCAPDispatcher();
}

} //inap
} //inman
} //smsc

