#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SAP_LIBSCCPCONNECTACCEPTOR_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SAP_LIBSCCPCONNECTACCEPTOR_HPP__

# include "eyeline/ss7na/common/sccp_sap/LibSccpConnectAcceptor.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace sap {

class LibSccpConnectAcceptor : public common::sccp_sap::LibSccpConnectAcceptor {
public:
  LibSccpConnectAcceptor(const std::string& acceptor_name, const std::string& host, in_port_t listen_port)
  : common::sccp_sap::LibSccpConnectAcceptor(acceptor_name, host, listen_port)
  {}
  virtual ~LibSccpConnectAcceptor() {}

  virtual common::io_dispatcher::Link* accept() const;
};

}}}}}

#endif
