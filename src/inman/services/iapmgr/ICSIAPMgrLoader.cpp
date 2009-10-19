#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/iapmgr/ICSIAPMgrProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderIAPManager(void)
{
  return new smsc::inman::iapmgr::ICSProdIAPManager();
}

} //inman
} //smsc

