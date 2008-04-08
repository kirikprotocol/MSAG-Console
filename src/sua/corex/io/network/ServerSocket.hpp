#ifndef __COREX_NETWORK_SERVERSOCKET_HPP__
# define __COREX_NETWORK_SERVERSOCKET_HPP__ 1

# include <sua/corex/io/network/Socket.hpp>
# include <string>

template <class LOCK>
class corex::io::IOObjectsPool_tmpl;

namespace corex {
namespace io {
namespace network {

class ServerSocket {
public:
  virtual void setReuseAddr(bool on) = 0;
  virtual void setNonBlocking(bool on) = 0;
  virtual Socket* accept() = 0;
  virtual std::string toString() const = 0;
  virtual void close() = 0;
protected:
  int getDescriptor();

  virtual int _getDescriptor() = 0;
  template <class LOCK>
  friend class corex::io::IOObjectsPool_tmpl; // to grant access to getDescriptor()
};

}}}

#endif
