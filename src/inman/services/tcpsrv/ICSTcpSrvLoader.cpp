#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/tcpsrv/ICSTcpSrvProd.hpp"

namespace smsc {
namespace inman {

ICSProducerAC * ICSLoaderTcpServer(void)
{
  return new smsc::inman::tcpsrv::ICSProdTcpServer();
}

} //inman
} //smsc

