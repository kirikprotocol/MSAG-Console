#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/smbill/ICSSmBillProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderSmBilling(void)
{
  return new smsc::inman::smbill::ICSProdSmBilling();
}

} //inman
} //smsc

