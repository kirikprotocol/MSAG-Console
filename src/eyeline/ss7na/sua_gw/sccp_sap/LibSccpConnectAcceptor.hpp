#ifndef __EYELINE_SS7NA_SUAGW_SCCPSAP_LIBSCCPCONNECTACCEPTOR_HPP__
# define __EYELINE_SS7NA_SUAGW_SCCPSAP_LIBSCCPCONNECTACCEPTOR_HPP__

# include "eyeline/ss7na/common/sccp_sap/LibSccpConnectAcceptor.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sccp_sap {

class LibSccpConnectAcceptor : public common::sccp_sap::LibSccpConnectAcceptor {
public:
  LibSccpConnectAcceptor(const std::string& acceptor_name, const std::string& host, in_port_t listen_port)
  : common::sccp_sap::LibSccpConnectAcceptor(acceptor_name, host, listen_port)
  {}

  virtual common::io_dispatcher::Link* accept() const;
};

}}}}

#endif
