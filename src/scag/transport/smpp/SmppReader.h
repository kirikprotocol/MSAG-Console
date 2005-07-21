#ifndef __SCAG_TRANSPORT_SMPP_SMPPREADER_H__
#define __SCAG_TRANSPORT_SMPP_SMPPREADER_H__

#include "SmppIOBase.h"

namespace scag{
namespace transport{
namespace smpp{

class SmppReader:public SmppIOBase
{
public:
  const char* taskName(){return "SmppReader";}
  int Execute();
  void onAddSocket(SmppSocket* sock)
  {
    mul.add(sock->getSocket());
  }
  void onDeleteSocket(SmppSocket* sock)
  {
    mul.remove(sock->getSocket());
  }
protected:
};


}//smpp
}//transport
}//scag


#endif
