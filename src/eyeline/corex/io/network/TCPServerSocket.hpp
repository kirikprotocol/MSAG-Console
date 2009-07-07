#ifndef __EYELINE_COREX_NETWORK_TCPSERVERSOCKET_HPP__
# define __EYELINE_COREX_NETWORK_TCPSERVERSOCKET_HPP__

# include "eyeline/corex/io/network/ServerSocket.hpp"
# include "eyeline/corex/io/network/TCPSocket.hpp"

namespace eyeline {
namespace corex {
namespace io {
namespace network {

class TCPServerSocket : public ServerSocket,
                        private TCPSocket {
public:
  explicit TCPServerSocket(in_port_t port);
  TCPServerSocket(const std::string& host, in_port_t port);

  using ServerSocket::setNonBlocking;

  virtual TCPSocket* accept();
  virtual void setReuseAddr(bool on);
  virtual void setNonBlocking(bool on);
  virtual std::string toString() const;
  virtual void close();
protected:
  virtual int _getDescriptor();
private:
  virtual void bind();
  virtual void listen();
};

}}}}

#endif
