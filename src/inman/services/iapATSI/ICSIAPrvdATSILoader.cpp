#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapATSI/ICSIAPrvdATSIProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderIAPrvdATSI(void)
{
    return new smsc::inman::iaprvd::atih::ICSProdIAPrvdATSI();
}

} //inman
} //smsc

