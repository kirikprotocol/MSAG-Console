#ifndef __EYELINE_COREX_IO_NETWORK_SCTPSERVERSOCKET_HPP__
# define __EYELINE_COREX_IO_NETWORK_SCTPSERVERSOCKET_HPP__

#ifndef __SunOS_5_9

# include <string>

# include "eyeline/corex/io/network/ServerSocket.hpp"
# include "eyeline/corex/io/network/SCTPSocket.hpp"

namespace eyeline {
namespace corex {
namespace io {
namespace network {

class SCTPServerSocket : public ServerSocket,
                         private SCTPSocket {
public:
  explicit SCTPServerSocket(in_port_t port);
  virtual SCTPSocket* accept();
  virtual void setReuseAddr(bool on);

  using IOObject::setNonBlocking;

  virtual void setNonBlocking(bool on);
  virtual std::string toString() const;
  virtual void close();
protected:
  virtual int _getDescriptor();
private:
  virtual void bind(/*const std::string& host, in_port_t port*/);
  virtual void listen();

};

}}}}

#endif

#endif
