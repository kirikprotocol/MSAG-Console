#ifndef __COREX_NETWORK_TCPSERVERSOCKET_HPP__
# define __COREX_NETWORK_TCPSERVERSOCKET_HPP__ 1

# include <sua/corex/io/network/ServerSocket.hpp>
# include <sua/corex/io/network/TCPSocket.hpp>

namespace corex {
namespace io {
namespace network {

class TCPServerSocket : public ServerSocket,
                        private TCPSocket {
public:
  explicit TCPServerSocket(in_port_t port);
  TCPServerSocket(const std::string& host, in_port_t port);
  virtual TCPSocket* accept();
  virtual void setReuseAddr(bool on);
  virtual void setNonBlocking(bool on);
  virtual std::string toString() const;
  virtual void close();
protected:
  virtual int _getDescriptor();
private:
  virtual void bind(/*const std::string& host, in_port_t port*/);
  virtual void listen();
};

}}}

#endif
