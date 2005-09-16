#ifndef __SCAG_SMPP_SMPPCOMMANDQUEUE_HPP__
#define __SCAG_SMPP_SMPPCOMMANDQUEUE_HPP__

#include "SmppTypes.h"

namespace scag{
namespace transport{
namespace smpp{

class SmppCommand;

class SmppCommandQueue{
public:
  virtual void putCommand(SmppBindType ct,const SmppCommand& cmd)=0;
  virtual bool getCommand(SmppCommand& cmd)=0;
};

}//smpp
}//transport
}//scag

#endif
