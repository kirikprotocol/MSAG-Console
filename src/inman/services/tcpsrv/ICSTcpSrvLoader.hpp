/* ************************************************************************* *
 * TCP Server service loader.
 * ************************************************************************* */
#ifndef __INMAN_ICS_TCPSERVER_LOADER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TCPSERVER_LOADER_HPP

#include "inman/services/ICSrvDefs.hpp"

namespace smsc {
namespace inman {

//Generic ICService loading function
extern ICSProducerAC * ICSLoaderTcpServer(void);

} //inman
} //smsc
#endif /* __INMAN_ICS_TCPSERVER_LOADER_HPP */

