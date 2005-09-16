#ifndef __SCAG_SMPP_SMPPCHANNEL_HPP__
#define __SCAG_SMPP_SMPPCHANNEL_HPP__

#include "SmppCommand.h"
#include "SmppTypes.h"

namespace scag{
namespace transport{
namespace smpp{

class SmppChannel{
public:
  virtual SmppBindType getBindType()const=0;
  virtual const char* getSystemId()const=0;
  virtual void putCommand(const SmppCommand& cmd)=0;
  void setUid(int argUid){uid=argUid;}
  int  getUid(){return uid;}
protected:
  int uid;
};

}//smpp
}//transport
}//scag

#endif
