#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapSRI/ICSIAPrvdSRIProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderIAPrvdSRI(void)
{
    return new smsc::inman::iaprvd::sri::ICSProdIAPrvdSRI();
}

} //inman
} //smsc

