#ifndef _SCAG_PVSS_SERVER_CONNECTION_H_
#define _SCAG_PVSS_SERVER_CONNECTION_H_

#include <vector>
#include <string>
#include "logger/Logger.h"

namespace smsc {
namespace core {
namespace network {
class Socket;
}
}
}

namespace scag2 {
namespace pvss  {

class Response;

class Connection {
public:
  virtual void sendResponse(const Response* resp) = 0;
  virtual smsc::core::network::Socket* getSocket() const = 0;
  virtual ~Connection() {};
};

}//pvss
}//scag2

#endif
  
 
