#ifndef __SCAG_TRANSPORT_SMPP_SMPPENTITY_H__
#define __SCAG_TRANSPORT_SMPP_SMPPENTITY_H__

#include "scag/transport/smpp/SmppChannel.h"
#include "scag/transport/smpp/SmppTypes.h"
#include "core/synchronization/Mutex.hpp"

namespace scag{
namespace transport{
namespace smpp{


struct SmppEntity{
  char systemId[32];
  int priority;
  SmppEntityType et;
  SmppBindType bt;
  smsc::core::synchronization::Mutex mtx;
  SmppChannel* channel;
  SmppChannel* recvChannel;
  SmppChannel* transChannel;
};

}//smpp
}//transport
}//scag

#endif
