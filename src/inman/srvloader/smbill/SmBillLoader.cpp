#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/ICSrvLoader.hpp"
#include "inman/services/smbill/ICSSmBillLoader.hpp"

namespace smsc {
namespace inman {

extern "C" ICSProducerAC * loadupICService(void)
{
    return ICSLoaderSmBilling();
}

} //inman
} //smsc

