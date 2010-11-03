#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/tcpsrv/ICSTcpSrvProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderTcpServer(void)
{
  return new smsc::inman::tcpsrv::ICSProdTcpServer();
}

} //inman
} //smsc

