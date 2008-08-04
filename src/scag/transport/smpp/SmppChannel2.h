#ifndef _SCAG_TRANSPORT_SMPP_SMPPCHANNEL_H
#define _SCAG_TRANSPORT_SMPP_SMPPCHANNEL_H

#include "SmppCommand2.h"
#include "SmppTypes2.h"

namespace scag2 {
namespace transport {
namespace smpp {

class SmppChannel{
public:
  virtual SmppBindType getBindType()const=0;
  virtual const char* getSystemId()const=0;

    virtual void putCommand(std::auto_ptr<SmppCommand> cmd)=0;
  virtual void disconnect()=0;
  void setUid(int argUid){uid=argUid;}
  int  getUid(){return uid;}
  virtual std::string getPeer() = 0;
protected:
  int uid;
};

}//smpp
}//transport
}//scag

#endif
