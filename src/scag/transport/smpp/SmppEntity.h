#ifndef __SCAG_TRANSPORT_SMPP_SMPPENTITY_H__
#define __SCAG_TRANSPORT_SMPP_SMPPENTITY_H__

#include "scag/transport/smpp/SmppChannel.h"
#include "scag/transport/smpp/SmppTypes.h"
#include "core/synchronization/Mutex.hpp"
#include "SmppManagerAdmin.h"

namespace scag{
namespace transport{
namespace smpp{


namespace sync=smsc::core::synchronization;

struct SmppEntity{
  SmppEntityInfo info;
  SmppBindType bt;
  sync::Mutex mtx;
  SmppChannel* channel;
  SmppChannel* recvChannel;
  SmppChannel* transChannel;
  int seq;

  SmppEntity()
  {
    bt=btNone;
    channel=0;
    recvChannel=0;
    transChannel=0;
    seq=0;
  }
  SmppEntity(const SmppEntityInfo& argInfo)
  {
    info=argInfo;
    bt=btNone;
    channel=0;
    recvChannel=0;
    transChannel=0;
    seq=0;
  }

  SmppBindType getBindType()const
  {
    return bt;
  }
  const char* getSystemId()const
  {
    return info.systemId;
  }
  void putCommand(const SmppCommand& cmd)
  {
    sync::MutexGuard mg(mtx);
    switch(bt)
    {
      case btNone:
        throw Exception("Attempt to putCommand to unbound enitity '%s'",info.systemId.c_str());
      case btReceiver:
        recvChannel->putCommand(cmd);
        break;
      case btTransmitter:
        transChannel->putCommand(cmd);
        break;
      case btRecvAndTrans:
        switch(cmd->get_commandId())
        {
        };
        break;
      case btTransceiver:
        channel->putCommand(cmd);
        break;
    }
  }
  void setUid(int argUid)
  {
    sync::MutexGuard mg(mtx);
    switch(bt)
    {
      case btNone:
        throw Exception("Failed to set uid of unbound entity '%s'",info.systemId.c_str());
      case btReceiver:
        recvChannel->setUid(argUid);
        break;
      case btTransmitter:
        transChannel->setUid(argUid);
        break;
      case btRecvAndTrans:
        recvChannel->setUid(argUid);
        transChannel->setUid(argUid);
        break;
      case btTransceiver:
        channel->setUid(argUid);
        break;
    }
  }
  int  getUid()
  {
    sync::MutexGuard mg(mtx);
    switch(bt)
    {
      case btNone:
        throw Exception("Failed to get uid of unbound entity '%s'",info.systemId.c_str());
      case btReceiver:
        return recvChannel->getUid();
      case btTransmitter:
        return transChannel->getUid();
      case btRecvAndTrans:
        return transChannel->getUid();
      case btTransceiver:
        return channel->getUid();
    }
  }

  int getNextSeq()
  {
    sync::MutexGuard mg(mtx);
    return ++seq;
  }
};

}//smpp
}//transport
}//scag

#endif
