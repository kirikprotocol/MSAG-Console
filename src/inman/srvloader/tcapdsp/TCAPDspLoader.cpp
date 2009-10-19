#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/ICSrvLoader.hpp"
#include "inman/services/tcapdsp/ICSTCDspLoader.hpp"

namespace smsc {
namespace inman {

//This is the TCAP Dispatcher Service dynamic library entry point
extern "C" ICSProducerAC * loadupICService(void)
{
    return ICSLoaderTCAPDispatcher();
}

} //inman
} //smsc

