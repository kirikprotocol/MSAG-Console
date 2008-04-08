#ifndef __COREX_IO_NETWORK_SCTPSERVERSOCKET_HPP__
# define __COREX_IO_NETWORK_SCTPSERVERSOCKET_HPP__ 1

# include <string>
# include <sua/corex/io/network/ServerSocket.hpp>
# include <sua/corex/io/network/SCTPSocket.hpp>

namespace corex {
namespace io {
namespace network {

class SCTPServerSocket : public ServerSocket,
                         private SCTPSocket {
public:
  explicit SCTPServerSocket(in_port_t port);
  virtual SCTPSocket* accept();
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
