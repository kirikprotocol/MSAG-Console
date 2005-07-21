#ifndef __SCAG_TRANSPORT_SMPP_SMPPCHANNELREGISTRATOR_H__
#define __SCAG_TRANSPORT_SMPP_SMPPCHANNELREGISTRATOR_H__

#include "SmppTypes.h"
#include "SmppChannel.h"

namespace scag{
namespace transport{
namespace smpp{

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
