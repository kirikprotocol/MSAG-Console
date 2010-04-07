#ifndef __EYELINE_SS7NA_COMMON_LIBSCCPCONNECTACCEPTOR_HPP__
# define __EYELINE_SS7NA_COMMON_LIBSCCPCONNECTACCEPTOR_HPP__

# include <string>
# include "eyeline/corex/io/network/TCPServerSocket.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectAcceptor.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

class LibSccpConnectAcceptor : public io_dispatcher::ConnectAcceptor {
public:
  LibSccpConnectAcceptor(const std::string& acceptor_name, const std::string& host, in_port_t listen_port);
  virtual ~LibSccpConnectAcceptor();

  virtual io_dispatcher::Link* accept() const;

  virtual const std::string& getName() const;

  virtual io_dispatcher::IOEvent* createIOEvent(const LinkId& link_id) const;

protected:
  virtual corex::io::network::TCPServerSocket* _getListenSocket() const;

private:
  corex::io::network::TCPServerSocket* _serverSocket;

  const std::string _acceptorName;
};

}}}}

#endif
