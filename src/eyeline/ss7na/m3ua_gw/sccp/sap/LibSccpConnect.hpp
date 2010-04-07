#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SAP_LIBSCCPCONNECT_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SAP_LIBSCCPCONNECT_HPP__

# include "eyeline/ss7na/common/sccp_sap/LibSccpConnect.hpp"
# include "eyeline/ss7na/m3ua_gw/types.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/sap/LibSccpConnectAcceptor.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace sap {

class LibSccpConnect : public common::sccp_sap::LibSccpConnect {
public:
  LibSccpConnect(corex::io::network::TCPSocket* socket,
                 const LibSccpConnectAcceptor* creator)
  : common::sccp_sap::LibSccpConnect(socket, creator)
  {}

  virtual common::TP* receive() {
    common::TP* tp = common::sccp_sap::LibSccpConnect::receive();
    tp->protocolClass = PROTOCOL_LIBSCCP;
    return tp;
  }
};

}}}}}

#endif
