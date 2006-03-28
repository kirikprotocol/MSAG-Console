#ifndef __SCAG_TRANSPORT_SMPP_SMPPENTITY_H__
#define __SCAG_TRANSPORT_SMPP_SMPPENTITY_H__

#include "scag/transport/smpp/SmppChannel.h"
#include "scag/transport/smpp/SmppTypes.h"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/IntHash.hpp"
#include "SmppManagerAdmin.h"

namespace scag{
namespace transport{
namespace smpp{

namespace buff=smsc::core::buffers;
namespace sync=smsc::core::synchronization;

struct SmppEntity
{
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
    delMappings();
  }
  ~SmppEntity() 
  {
    delMappings();
  }
  
  int getUSR(int umr)
  {
    sync::MutexGuard guard(mappingLock);
    int* usr = UMRtoUSR.GetPtr(umr);
    return ((usr) ? *usr:-1);
  }
  int getUMR(int usr)
  {
    sync::MutexGuard guard(mappingLock);
    int* umr = USRtoUMR.GetPtr(usr);
    return ((umr) ? *umr:-1);
  }
  void setMapping(int umr, int usr)
  {
    sync::MutexGuard guard(mappingLock);
    int* ptr = USRtoUMR.GetPtr(usr);
    if (ptr) *ptr=umr;
    else USRtoUMR.Insert(usr, umr);
    ptr = UMRtoUSR.GetPtr(umr);
    if (ptr) *ptr=usr;
    else UMRtoUSR.Insert(umr, usr);
  }
  bool delUSRMapping(int usr)
  {
    sync::MutexGuard guard(mappingLock);
    int* ptr = USRtoUMR.GetPtr(usr);
    if (!ptr) return false;
    UMRtoUSR.Delete(*ptr); USRtoUMR.Delete(usr);
    return true;
  }
  bool delUMRMapping(int umr)
  {
    sync::MutexGuard guard(mappingLock);
    int* ptr = UMRtoUSR.GetPtr(umr);
    if (!ptr) return false;
    USRtoUMR.Delete(*ptr); UMRtoUSR.Delete(umr);
    return true;
  }
  void delMappings()
  {
    sync::MutexGuard guard(mappingLock);
    UMRtoUSR.Empty(); USRtoUMR.Empty();
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

protected:

  sync::Mutex  mappingLock;
  buff::IntHash<int> UMRtoUSR;
  buff::IntHash<int> USRtoUMR;
  
};

}//smpp
}//transport
}//scag

#endif
