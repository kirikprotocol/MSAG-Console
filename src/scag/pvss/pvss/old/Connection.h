#ifndef _SCAG_PVSS_SERVER_CONNECTION_H_
#define _SCAG_PVSS_SERVER_CONNECTION_H_

#include "logger/Logger.h"
#include "scag/pvss/api/core/PvssSocketBase.h"

namespace scag2 {
namespace pvss  {

class Response;

class Connection : public core::PvssSocketBase
{
protected:
    explicit Connection( smsc::core::network::Socket* sock ) :
    core::PvssSocketBase(sock) {}

public:
  virtual void sendResponse(const Response* resp) = 0;
};

}//pvss
}//scag2

#endif
  
 
