#ifndef _SCAG_PVSS_SERVER_CONNECTION_H_
#define _SCAG_PVSS_SERVER_CONNECTION_H_

namespace scag2 {
namespace pvss  {

class Connection {
public:
  virtual void sendResponse(const char* data, uint32_t dataSize) = 0;
  virtual ~Connection() {};
};

}//pvss
}//scag2

#endif
  
 
