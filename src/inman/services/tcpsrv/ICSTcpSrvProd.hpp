/* ************************************************************************* *
 * TCP Server service producer.
 * ************************************************************************* */
#ifndef __INMAN_ICS_TCPSERVER_PRODUCER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TCPSERVER_PRODUCER_HPP

#include "inman/services/tcpsrv/ICSTcpSrv.hpp"
#include "inman/services/tcpsrv/TCPSrvCfgReader.hpp"
#include "inman/services/ICSXcfProducer.hpp"

namespace smsc {
namespace inman {
namespace tcpsrv {

//TCP server service producer.
class ICSProdTcpServer : public 
    ICSProducerXcfAC_T<ICSTcpServer, ICSTcpSrvCfgReader, ServSocketCFG> {
public:
    ICSProdTcpServer()
        : ICSProducerXcfAC_T<ICSTcpServer, ICSTcpSrvCfgReader, ServSocketCFG>
            (ICSIdent::icsIdTCPServer)
    { }
    ~ICSProdTcpServer()
    { }
};

} //tcpsrv
} //inman
} //smsc
#endif /* __INMAN_ICS_TCPSERVER_PRODUCER_HPP */

