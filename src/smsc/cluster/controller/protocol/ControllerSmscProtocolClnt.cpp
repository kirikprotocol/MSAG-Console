#include "ControllerSmscProtocolClnt.hpp"
#include "messages/ApplyRoutes.hpp"
#include "messages/ApplyReschedule.hpp"
#include "messages/ApplyLocaleResource.hpp"
#include "messages/ApplyTimeZones.hpp"
#include "messages/ApplyFraudControl.hpp"
#include "messages/ApplyMapLimits.hpp"
#include "messages/ApplySnmp.hpp"
#include "messages/UpdateProfile.hpp"
#include "messages/DeleteProfile.hpp"
#include "messages/CancelSms.hpp"
#include "messages/MscAdd.hpp"
#include "messages/MscRemove.hpp"
#include "messages/SmeAdd.hpp"
#include "messages/SmeUpdate.hpp"
#include "messages/SmeRemove.hpp"
#include "messages/SmeStatus.hpp"
#include "messages/SmeDisconnect.hpp"
#include "messages/LoggerGetCategories.hpp"
#include "messages/LoggerSetCategories.hpp"
#include "messages/AclRemove.hpp"
#include "messages/AclCreate.hpp"
#include "messages/AclUpdate.hpp"
#include "messages/AclRemoveAddresses.hpp"
#include "messages/AclAddAddresses.hpp"
#include "messages/CgmAddGroup.hpp"
#include "messages/CgmDeleteGroup.hpp"
#include "messages/CgmAddAddr.hpp"
#include "messages/CgmDelAddr.hpp"
#include "messages/CgmAddAbonent.hpp"
#include "messages/CgmDelAbonent.hpp"
#include "messages/AliasAdd.hpp"
#include "messages/AliasDel.hpp"
#include "messages/UpdateProfileAbntResp.hpp"
#include "messages/GetServicesStatus.hpp"
#include "messages/DisconnectService.hpp"
#include "messages/LockConfigResp.hpp"
#include "messages/GetSmscConfigsState.hpp"

namespace smsc {
namespace cluster {
namespace controller {
namespace protocol {

void ControllerSmscProtocol::assignHandler(ControllerHandler* newHandler)
{
  handler=newHandler;
}

void ControllerSmscProtocol::decodeAndHandleMessage(const char* buf,size_t sz)
{
  eyeline::protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss);
}

void ControllerSmscProtocol::decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss)
{
  uint32_t tag=ss.readInt32();
  uint32_t seq=ss.readInt32();
  switch(tag)
  {
    case tag_ApplyRoutes:
    {
      messages::ApplyRoutes msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyReschedule:
    {
      messages::ApplyReschedule msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyLocaleResource:
    {
      messages::ApplyLocaleResource msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyTimeZones:
    {
      messages::ApplyTimeZones msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyFraudControl:
    {
      messages::ApplyFraudControl msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyMapLimits:
    {
      messages::ApplyMapLimits msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplySnmp:
    {
      messages::ApplySnmp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UpdateProfile:
    {
      messages::UpdateProfile msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DeleteProfile:
    {
      messages::DeleteProfile msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CancelSms:
    {
      messages::CancelSms msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscAdd:
    {
      messages::MscAdd msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscRemove:
    {
      messages::MscRemove msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeAdd:
    {
      messages::SmeAdd msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeUpdate:
    {
      messages::SmeUpdate msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeRemove:
    {
      messages::SmeRemove msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeStatus:
    {
      messages::SmeStatus msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeDisconnect:
    {
      messages::SmeDisconnect msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LoggerGetCategories:
    {
      messages::LoggerGetCategories msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LoggerSetCategories:
    {
      messages::LoggerSetCategories msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclRemove:
    {
      messages::AclRemove msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclCreate:
    {
      messages::AclCreate msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclUpdate:
    {
      messages::AclUpdate msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclRemoveAddresses:
    {
      messages::AclRemoveAddresses msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclAddAddresses:
    {
      messages::AclAddAddresses msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddGroup:
    {
      messages::CgmAddGroup msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDeleteGroup:
    {
      messages::CgmDeleteGroup msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddAddr:
    {
      messages::CgmAddAddr msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDelAddr:
    {
      messages::CgmDelAddr msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddAbonent:
    {
      messages::CgmAddAbonent msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDelAbonent:
    {
      messages::CgmDelAbonent msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AliasAdd:
    {
      messages::AliasAdd msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AliasDel:
    {
      messages::AliasDel msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UpdateProfileAbntResp:
    {
      messages::UpdateProfileAbntResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetServicesStatus:
    {
      messages::GetServicesStatus msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DisconnectService:
    {
      messages::DisconnectService msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LockConfigResp:
    {
      messages::LockConfigResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetSmscConfigsState:
    {
      messages::GetSmscConfigsState msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    default:
      throw eyeline::protogen::framework::UnhandledMessage(tag);
  }
}


}
}
}
}
