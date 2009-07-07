#ifndef __EYELINE_COREX_NETWORK_SOCKET_HPP__
# define __EYELINE_COREX_NETWORK_SOCKET_HPP__

# include <sys/types.h>
# include <netinet/in.h>
# include <string>

# include "util/Exception.hpp"
# include "eyeline/corex/io/IOStreams.hpp"
# include "eyeline/corex/io/IOObject.hpp"

namespace eyeline {
namespace corex {
namespace io {
namespace network {

class Socket : public corex::io::IOObject {
public:
  virtual void setNoDelay(bool on) = 0;
  virtual void setReuseAddr(bool on) = 0;
  virtual void setSoLinger(bool on, int timeout=0) = 0;
};

}}}}

#endif
