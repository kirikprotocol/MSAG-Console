#ifndef __SCAG_MTPERS_CONNECTION_H__
#define __SCAG_MTPERS_CONNECTION_H__

namespace scag { namespace mtpers {

class Connection {
public:
  virtual void sendResponse(const char* data, uint32_t dataSize) = 0;
  virtual ~Connection() {};
};

}//mtpers
}//scag

#endif
  
 
