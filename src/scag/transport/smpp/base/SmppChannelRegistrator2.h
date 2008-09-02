#ifndef _SCAG_TRANSPORT_SMPP_SMPPCHANNELREGISTRATOR2_H
#define _SCAG_TRANSPORT_SMPP_SMPPCHANNELREGISTRATOR2_H

#include "scag/transport/smpp/SmppTypes.h"
#include "SmppChannel2.h"

namespace scag2 {
namespace transport {
namespace smpp {

enum RegistrationAttemptResult{
  rarOk,
  rarFailed,
  rarAlready
};

struct SmppChannelRegistrator{
  virtual int registerSmeChannel(const char* sysId,const char* pwd,SmppBindType bt,SmppChannel* ch)=0;
  virtual int registerSmscChannel(SmppChannel* ch)=0;
  virtual void unregisterChannel(SmppChannel* ch)=0;
};

}//smpp
}//transport
}//scag


#endif
