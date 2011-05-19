#include "ControllerProtocol.hpp"
#include "messages/TraceRoute.hpp"
#include "messages/CheckRoutes.hpp"
#include "messages/LookupProfile.hpp"
#include "messages/AclGet.hpp"
#include "messages/AclList.hpp"
#include "messages/CgmCheck.hpp"
#include "messages/CgmListAbonents.hpp"
#include "messages/GetServicesStatus.hpp"
#include "messages/DisconnectService.hpp"
#include "messages/LockConfig.hpp"
#include "messages/UnlockConfig.hpp"
#include "messages/GetConfigsState.hpp"
#include "messages/RegisterAsLoadBalancer.hpp"
#include "messages/RegisterAsWebapp.hpp"
#include "messages/RegisterAsSmsc.hpp"
#include "messages/UpdateProfileAbnt.hpp"
#include "messages/GetSmscConfigsStateResp.hpp"

namespace eyeline {
namespace clustercontroller {
namespace protocol {

void ControllerProtocol::assignHandler(ControllerProtocolHandler* newHandler)
{
  handler=newHandler;
}

void ControllerProtocol::decodeAndHandleMessage(const char* buf,size_t sz)
{
  eyeline::protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss);
}

void ControllerProtocol::decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss)
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
    case tag_ApplyNetProfiles:
    {
      messages::ApplyNetProfiles msg;
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
    case tag_TraceRoute:
    {
      messages::TraceRoute msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CheckRoutes:
    {
      messages::CheckRoutes msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LookupProfile:
    {
      messages::LookupProfile msg;
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
    case tag_AclGet:
    {
      messages::AclGet msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclList:
    {
      messages::AclList msg;
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
    case tag_AclLookup:
    {
      messages::AclLookup msg;
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
    case tag_CgmCheck:
    {
      messages::CgmCheck msg;
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
    case tag_CgmListAbonents:
    {
      messages::CgmListAbonents msg;
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
    case tag_LockConfig:
    {
      messages::LockConfig msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UnlockConfig:
    {
      messages::UnlockConfig msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetConfigsState:
    {
      messages::GetConfigsState msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_RegisterAsLoadBalancer:
    {
      messages::RegisterAsLoadBalancer msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_RegisterAsWebapp:
    {
      messages::RegisterAsWebapp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_RegisterAsSmsc:
    {
      messages::RegisterAsSmsc msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyRoutesResp:
    {
      messages::ApplyRoutesResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyRescheduleResp:
    {
      messages::ApplyRescheduleResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyLocaleResourceResp:
    {
      messages::ApplyLocaleResourceResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyTimeZonesResp:
    {
      messages::ApplyTimeZonesResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyFraudControlResp:
    {
      messages::ApplyFraudControlResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyMapLimitsResp:
    {
      messages::ApplyMapLimitsResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyNetProfilesResp:
    {
      messages::ApplyNetProfilesResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplySnmpResp:
    {
      messages::ApplySnmpResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UpdateProfileResp:
    {
      messages::UpdateProfileResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DeleteProfileResp:
    {
      messages::DeleteProfileResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CancelSmsResp:
    {
      messages::CancelSmsResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscAddResp:
    {
      messages::MscAddResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscRemoveResp:
    {
      messages::MscRemoveResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeAddResp:
    {
      messages::SmeAddResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeUpdateResp:
    {
      messages::SmeUpdateResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeRemoveResp:
    {
      messages::SmeRemoveResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeStatusResp:
    {
      messages::SmeStatusResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeDisconnectResp:
    {
      messages::SmeDisconnectResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LoggerGetCategoriesResp:
    {
      messages::LoggerGetCategoriesResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LoggerSetCategoriesResp:
    {
      messages::LoggerSetCategoriesResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclRemoveResp:
    {
      messages::AclRemoveResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclCreateResp:
    {
      messages::AclCreateResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclUpdateResp:
    {
      messages::AclUpdateResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclRemoveAddressesResp:
    {
      messages::AclRemoveAddressesResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclAddAddressesResp:
    {
      messages::AclAddAddressesResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddGroupResp:
    {
      messages::CgmAddGroupResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDeleteGroupResp:
    {
      messages::CgmDeleteGroupResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddAddrResp:
    {
      messages::CgmAddAddrResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDelAddrResp:
    {
      messages::CgmDelAddrResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddAbonentResp:
    {
      messages::CgmAddAbonentResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDelAbonentResp:
    {
      messages::CgmDelAbonentResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AliasAddResp:
    {
      messages::AliasAddResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AliasDelResp:
    {
      messages::AliasDelResp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UpdateProfileAbnt:
    {
      messages::UpdateProfileAbnt msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetSmscConfigsStateResp:
    {
      messages::GetSmscConfigsStateResp msg;
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
