#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/iapSRI/ICSIAPrvdSRIProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderIAPrvdSRI(void)
{
    return new smsc::inman::iaprvd::sri::ICSProdIAPrvdSRI();
}

} //inman
} //smsc

