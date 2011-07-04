#ifndef __SCAG_TRANSPORT_SMPP_SMPPREADER2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPREADER2_H__

#include "SmppIOBase2.h"

namespace scag2 {
namespace transport {
namespace smpp {

class SmppReader:public SmppIOBase
{
public:
  const char* taskName(){return "SmppReader";}
  int Execute();
  void onAddSocket(SmppSocket& sock)
  {
    MutexGuard mg(mulMtx);
    mul.add(sock.getSocket());
  }
  void onDeleteSocket(SmppSocket& sock)
  {
    MutexGuard mg(mulMtx);
    mul.remove(sock.getSocket());
  }
protected:
  sync::Mutex mulMtx;
};


}//smpp
}//transport
}//scag


#endif
