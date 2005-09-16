#ifndef __SCAG_TRANSPORT_SMPP_SMPPENTITY_H__
#define __SCAG_TRANSPORT_SMPP_SMPPENTITY_H__

#include "scag/transport/smpp/SmppChannel.h"
#include "scag/transport/smpp/SmppTypes.h"
#include "core/synchronization/Mutex.hpp"
#include "SmppManagerAdmin.h"

namespace scag{
namespace transport{
namespace smpp{


struct SmppEntity{
  SmppEntityInfo info;
  SmppBindType bt;
  smsc::core::synchronization::Mutex mtx;
  SmppChannel* channel;
  SmppChannel* recvChannel;
  SmppChannel* transChannel;

  SmppEntity()
  {
    bt=btNone;
    channel=0;
    recvChannel=0;
    transChannel=0;
  }
  SmppEntity(const SmppEntityInfo& argInfo)
  {
    info=argInfo;
    bt=btNone;
    channel=0;
    recvChannel=0;
    transChannel=0;
  }
};

}//smpp
}//transport
}//scag

#endif
