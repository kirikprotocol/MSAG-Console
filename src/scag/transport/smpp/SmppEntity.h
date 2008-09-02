#ifndef __SCAG_TRANSPORT_SMPP_SMPPENTITY_H__
#define __SCAG_TRANSPORT_SMPP_SMPPENTITY_H__

#include "scag/transport/smpp/SmppTypes.h"
#include "scag/transport/smpp/SmppChannel.h"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/Hash.hpp"
#include "SmppManagerAdmin.h"
#include "util/timeslotcounter.hpp"

namespace scag{
namespace transport{
namespace smpp{

namespace buff=smsc::core::buffers;
namespace sync=smsc::core::synchronization;

struct UMRUSR
{
  buff::IntHash<int> UMRtoUSR;
  buff::IntHash<int> USRtoUMR;
};

struct SmppEntity
{
  SmppEntityInfo info;
  SmppBindType bt;
  sync::Mutex mtx;
  SmppChannel* channel;
  SmppChannel* recvChannel;
  SmppChannel* transChannel;
  sync::Mutex seqMtx;
  int seq;
  uint16_t slicingSeq;
  bool connected;
  smsc::util::TimeSlotCounter<> incCnt;
  sync::Mutex cntMtx;
  int queueCount;

  SmppEntity():incCnt(5)
  {
    bt=btNone;
    channel=0;
    recvChannel=0;
    transChannel=0;
    seq=0;
    connected = false;
    queueCount=0;
    slicingSeq = 0;
  }
  SmppEntity(const SmppEntityInfo& argInfo):incCnt(5)
  {
    info=argInfo;
    bt=btNone;
    channel=0;
    recvChannel=0;
    transChannel=0;
    seq=1;
    queueCount=0;
    connected = false;
    slicingSeq = 0;
  }
  ~SmppEntity()
  {
    delMapping();
  }

  int getUSR(const Address& abonent, int umr)
  {
    std::string abs = abonent.toString();
    const char* ab = abs.c_str();
    __require__(ab);

    sync::MutexGuard guard(mappingLock);
    UMRUSR* umrusr = mapping.GetPtr(ab);
    if (!umrusr) return -1;
    int* usr = umrusr->UMRtoUSR.GetPtr(umr);
    return ((usr) ? *usr:-1);
  }
  int getUMR(const Address& abonent, int usr)
  {
    std::string abs = abonent.toString();
    const char* ab = abs.c_str();
    __require__(ab);

    sync::MutexGuard guard(mappingLock);
    UMRUSR* umrusr = mapping.GetPtr(ab);
    if (!umrusr) return -1;
    int* umr = umrusr->USRtoUMR.GetPtr(usr);
    return ((umr) ? *umr:-1);
  }
  void setMapping(const Address& abonent, int umr, int usr)
  {
    std::string abs = abonent.toString();
    const char* ab = abs.c_str();
    __require__(ab);

    sync::MutexGuard guard(mappingLock);
    UMRUSR* umrusr = mapping.GetPtr(ab);
    if (!umrusr) {
        mapping.Insert(ab, UMRUSR());
        umrusr = mapping.GetPtr(ab);
    }
    int* ptr = umrusr->USRtoUMR.GetPtr(usr);
    if (ptr) *ptr=umr;
    else umrusr->USRtoUMR.Insert(usr, umr);
    ptr = umrusr->UMRtoUSR.GetPtr(umr);
    if (ptr) *ptr=usr;
    else umrusr->UMRtoUSR.Insert(umr, usr);
  }
  bool delUSRMapping(const Address& abonent, int usr)
  {
    std::string abs = abonent.toString();
    const char* ab = abs.c_str();
    __require__(ab);

    sync::MutexGuard guard(mappingLock);
    UMRUSR* umrusr = mapping.GetPtr(ab);
    if (!umrusr) return false;
    int* ptr = umrusr->USRtoUMR.GetPtr(usr);
    if (!ptr) return false;
    umrusr->UMRtoUSR.Delete(*ptr); umrusr->USRtoUMR.Delete(usr);
    if ((umrusr->UMRtoUSR.Count() <= 0) && (umrusr->USRtoUMR.Count() <= 0)) mapping.Delete(ab);
    return true;
  }
  bool delUMRMapping(const Address& abonent, int umr)
  {
    std::string abs = abonent.toString();
    const char* ab = abs.c_str();
    __require__(ab);

    sync::MutexGuard guard(mappingLock);
    UMRUSR* umrusr = mapping.GetPtr(ab);
    if (!umrusr) return false;
    int* ptr = umrusr->UMRtoUSR.GetPtr(umr);
    if (!ptr) return false;
    umrusr->USRtoUMR.Delete(*ptr); umrusr->UMRtoUSR.Delete(umr);
    if ((umrusr->UMRtoUSR.Count() <= 0) && (umrusr->USRtoUMR.Count() <= 0)) mapping.Delete(ab);
    return true;
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
    sync::MutexGuard mg(seqMtx);
    return ++seq;
  }
  
  int getNextSlicingSeq()
  {
    sync::MutexGuard mg(seqMtx);
    return ++slicingSeq;
  }

  int getQueueCount()
  {
    sync::MutexGuard mg(cntMtx);
    return queueCount;
  }

  void incQueueCount()
  {
    sync::MutexGuard mg(cntMtx);
    queueCount++;
  }

  void decQueueCount()
  {
    sync::MutexGuard mg(cntMtx);
    queueCount--;
  }

protected:

  inline void delMapping()
  {
    sync::MutexGuard guard(mappingLock);
    mapping.Empty();
  }

  sync::Mutex    mappingLock;
  buff::Hash<UMRUSR> mapping;
};

}//smpp
}//transport
}//scag

#endif
