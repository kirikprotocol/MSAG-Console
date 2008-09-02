#ifndef __SCAG_TRANSPORT_SMPP_SMPPSMINTERFACE2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSMINTERFACE2_H__

namespace scag2 {
namespace transport {
namespace smpp {

class SmppSocket;

struct SmppSMInterface{
  virtual void registerSocket(SmppSocket* sock)=0;
  virtual void unregisterSocket(SmppSocket* sock)=0;
};


}//smpp
}//transport
}//scag


#endif
