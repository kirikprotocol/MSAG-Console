#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/smbill/ICSSmBillProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderSmBilling(void)
{
  return new smsc::inman::smbill::ICSProdSmBilling();
}

} //inman
} //smsc

