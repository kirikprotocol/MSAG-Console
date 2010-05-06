#include "ControllerProtocol.hpp"
#include "messages/ApplyRoutes.hpp"
#include "messages/ApplyReschedule.hpp"
#include "messages/ApplyLocaleResource.hpp"
#include "messages/ApplyTimeZones.hpp"
#include "messages/ApplyFraudControl.hpp"
#include "messages/ApplyMapLimits.hpp"
#include "messages/ApplySnmp.hpp"
#include "messages/TraceRoute.hpp"
#include "messages/LoadRoutes.hpp"
#include "messages/LookupProfile.hpp"
#include "messages/LookupProfileEx.hpp"
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
#include "messages/AclGet.hpp"
#include "messages/AclList.hpp"
#include "messages/AclRemove.hpp"
#include "messages/AclCreate.hpp"
#include "messages/AclUpdate.hpp"
#include "messages/AclLookup.hpp"
#include "messages/AclRemoveAddresses.hpp"
#include "messages/AclAddAddresses.hpp"
#include "messages/CgmAddGroup.hpp"
#include "messages/CgmDeleteGroup.hpp"
#include "messages/CgmAddAddr.hpp"
#include "messages/CgmCheck.hpp"
#include "messages/CgmDelAddr.hpp"
#include "messages/CgmAddAbonent.hpp"
#include "messages/CgmDelAbonent.hpp"
#include "messages/CgmListAbonents.hpp"
#include "messages/AliasAdd.hpp"
#include "messages/AliasDel.hpp"
#include "messages/UpdateProfileAbntResp.hpp"
#include "messages/GetServicesStatus.hpp"
#include "messages/DisconnectService.hpp"
#include "messages/LockConfig.hpp"
#include "messages/UnlockConfig.hpp"
#include "messages/RegisterAsLoadBalancer.hpp"
#include "messages/RegisterAsWebapp.hpp"
#include "messages/RegisterAsSmsc.hpp"
#include "messages/ApplyRoutesResp.hpp"
#include "messages/ApplyRescheduleResp.hpp"
#include "messages/ApplyLocaleResourceResp.hpp"
#include "messages/ApplyTimeZonesResp.hpp"
#include "messages/ApplyFraudControlResp.hpp"
#include "messages/ApplyMapLimitsResp.hpp"
#include "messages/ApplySnmpResp.hpp"
#include "messages/TraceRouteResp.hpp"
#include "messages/LoadRoutesResp.hpp"
#include "messages/LookupProfileResp.hpp"
#include "messages/LookupProfileExResp.hpp"
#include "messages/UpdateProfileResp.hpp"
#include "messages/DeleteProfileResp.hpp"
#include "messages/CancelSmsResp.hpp"
#include "messages/MscAddResp.hpp"
#include "messages/MscRemoveResp.hpp"
#include "messages/SmeAddResp.hpp"
#include "messages/SmeUpdateResp.hpp"
#include "messages/SmeRemoveResp.hpp"
#include "messages/SmeStatusResp.hpp"
#include "messages/SmeDisconnectResp.hpp"
#include "messages/LoggerGetCategoriesResp.hpp"
#include "messages/LoggerSetCategoriesResp.hpp"
#include "messages/AclGetResp.hpp"
#include "messages/AclListResp.hpp"
#include "messages/AclRemoveResp.hpp"
#include "messages/AclCreateResp.hpp"
#include "messages/AclUpdateResp.hpp"
#include "messages/AclLookupResp.hpp"
#include "messages/AclRemoveAddressesResp.hpp"
#include "messages/AclAddAddressesResp.hpp"
#include "messages/CgmAddGroupResp.hpp"
#include "messages/CgmDeleteGroupResp.hpp"
#include "messages/CgmAddAddrResp.hpp"
#include "messages/CgmCheckResp.hpp"
#include "messages/CgmDelAddrResp.hpp"
#include "messages/CgmAddAbonentResp.hpp"
#include "messages/CgmDelAbonentResp.hpp"
#include "messages/CgmListAbonentsResp.hpp"
#include "messages/AliasAddResp.hpp"
#include "messages/AliasDelResp.hpp"
#include "messages/UpdateProfileAbnt.hpp"
#include "messages/GetServicesStatusResp.hpp"
#include "messages/DisconnectServiceResp.hpp"
#include "messages/LockConfigResp.hpp"

namespace eyeline {
namespace clustercontroller {
namespace protocol {

void ControllerProtocol::assignHandler(ControllerProtocolHandler* newHandler)
{
  handler=newHandler;
}

void ControllerProtocol::decodeAndHandleMessage(const char* buf,size_t sz)
{
  protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss);
}

void ControllerProtocol::decodeAndHandleMessage(protogen::framework::SerializerBuffer& ss)
{
  uint32_t tag=ss.readInt32();
  uint32_t seq=ss.readInt32();
  switch(tag)
  {
    case tag_ApplyRoutes:
    {
      messages::ApplyRoutes msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyReschedule:
    {
      messages::ApplyReschedule msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyLocaleResource:
    {
      messages::ApplyLocaleResource msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyTimeZones:
    {
      messages::ApplyTimeZones msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyFraudControl:
    {
      messages::ApplyFraudControl msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyMapLimits:
    {
      messages::ApplyMapLimits msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplySnmp:
    {
      messages::ApplySnmp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_TraceRoute:
    {
      messages::TraceRoute msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LoadRoutes:
    {
      messages::LoadRoutes msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LookupProfile:
    {
      messages::LookupProfile msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LookupProfileEx:
    {
      messages::LookupProfileEx msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UpdateProfile:
    {
      messages::UpdateProfile msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DeleteProfile:
    {
      messages::DeleteProfile msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CancelSms:
    {
      messages::CancelSms msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscAdd:
    {
      messages::MscAdd msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscRemove:
    {
      messages::MscRemove msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeAdd:
    {
      messages::SmeAdd msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeUpdate:
    {
      messages::SmeUpdate msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeRemove:
    {
      messages::SmeRemove msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeStatus:
    {
      messages::SmeStatus msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeDisconnect:
    {
      messages::SmeDisconnect msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LoggerGetCategories:
    {
      messages::LoggerGetCategories msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LoggerSetCategories:
    {
      messages::LoggerSetCategories msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclGet:
    {
      messages::AclGet msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclList:
    {
      messages::AclList msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclRemove:
    {
      messages::AclRemove msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclCreate:
    {
      messages::AclCreate msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclUpdate:
    {
      messages::AclUpdate msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclLookup:
    {
      messages::AclLookup msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclRemoveAddresses:
    {
      messages::AclRemoveAddresses msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclAddAddresses:
    {
      messages::AclAddAddresses msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddGroup:
    {
      messages::CgmAddGroup msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDeleteGroup:
    {
      messages::CgmDeleteGroup msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddAddr:
    {
      messages::CgmAddAddr msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmCheck:
    {
      messages::CgmCheck msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDelAddr:
    {
      messages::CgmDelAddr msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddAbonent:
    {
      messages::CgmAddAbonent msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDelAbonent:
    {
      messages::CgmDelAbonent msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmListAbonents:
    {
      messages::CgmListAbonents msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AliasAdd:
    {
      messages::AliasAdd msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AliasDel:
    {
      messages::AliasDel msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UpdateProfileAbntResp:
    {
      messages::UpdateProfileAbntResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetServicesStatus:
    {
      messages::GetServicesStatus msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DisconnectService:
    {
      messages::DisconnectService msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LockConfig:
    {
      messages::LockConfig msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UnlockConfig:
    {
      messages::UnlockConfig msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_RegisterAsLoadBalancer:
    {
      messages::RegisterAsLoadBalancer msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_RegisterAsWebapp:
    {
      messages::RegisterAsWebapp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_RegisterAsSmsc:
    {
      messages::RegisterAsSmsc msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyRoutesResp:
    {
      messages::ApplyRoutesResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyRescheduleResp:
    {
      messages::ApplyRescheduleResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyLocaleResourceResp:
    {
      messages::ApplyLocaleResourceResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyTimeZonesResp:
    {
      messages::ApplyTimeZonesResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyFraudControlResp:
    {
      messages::ApplyFraudControlResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplyMapLimitsResp:
    {
      messages::ApplyMapLimitsResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ApplySnmpResp:
    {
      messages::ApplySnmpResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_TraceRouteResp:
    {
      messages::TraceRouteResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LoadRoutesResp:
    {
      messages::LoadRoutesResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LookupProfileResp:
    {
      messages::LookupProfileResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LookupProfileExResp:
    {
      messages::LookupProfileExResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UpdateProfileResp:
    {
      messages::UpdateProfileResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DeleteProfileResp:
    {
      messages::DeleteProfileResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CancelSmsResp:
    {
      messages::CancelSmsResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscAddResp:
    {
      messages::MscAddResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscRemoveResp:
    {
      messages::MscRemoveResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeAddResp:
    {
      messages::SmeAddResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeUpdateResp:
    {
      messages::SmeUpdateResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeRemoveResp:
    {
      messages::SmeRemoveResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeStatusResp:
    {
      messages::SmeStatusResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SmeDisconnectResp:
    {
      messages::SmeDisconnectResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LoggerGetCategoriesResp:
    {
      messages::LoggerGetCategoriesResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LoggerSetCategoriesResp:
    {
      messages::LoggerSetCategoriesResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclGetResp:
    {
      messages::AclGetResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclListResp:
    {
      messages::AclListResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclRemoveResp:
    {
      messages::AclRemoveResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclCreateResp:
    {
      messages::AclCreateResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclUpdateResp:
    {
      messages::AclUpdateResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclLookupResp:
    {
      messages::AclLookupResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclRemoveAddressesResp:
    {
      messages::AclRemoveAddressesResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AclAddAddressesResp:
    {
      messages::AclAddAddressesResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddGroupResp:
    {
      messages::CgmAddGroupResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDeleteGroupResp:
    {
      messages::CgmDeleteGroupResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddAddrResp:
    {
      messages::CgmAddAddrResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmCheckResp:
    {
      messages::CgmCheckResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDelAddrResp:
    {
      messages::CgmDelAddrResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmAddAbonentResp:
    {
      messages::CgmAddAbonentResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmDelAbonentResp:
    {
      messages::CgmDelAbonentResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_CgmListAbonentsResp:
    {
      messages::CgmListAbonentsResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AliasAddResp:
    {
      messages::AliasAddResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_AliasDelResp:
    {
      messages::AliasDelResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UpdateProfileAbnt:
    {
      messages::UpdateProfileAbnt msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetServicesStatusResp:
    {
      messages::GetServicesStatusResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DisconnectServiceResp:
    {
      messages::DisconnectServiceResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LockConfigResp:
    {
      messages::LockConfigResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    default:
      throw protogen::framework::UnhandledMessage(tag);
  }
}

void ControllerProtocol::encodeMessage(const messages::ApplyRoutes& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyRoutes);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyReschedule& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyReschedule);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyLocaleResource& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyLocaleResource);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyTimeZones& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyTimeZones);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyFraudControl& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyFraudControl);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyMapLimits& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyMapLimits);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplySnmp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplySnmp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::TraceRoute& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_TraceRoute);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LoadRoutes& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LoadRoutes);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LookupProfile& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LookupProfile);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LookupProfileEx& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LookupProfileEx);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::UpdateProfile& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_UpdateProfile);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DeleteProfile& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DeleteProfile);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CancelSms& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CancelSms);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::MscAdd& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscAdd);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::MscRemove& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscRemove);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::SmeAdd& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeAdd);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::SmeUpdate& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeUpdate);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::SmeRemove& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeRemove);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::SmeStatus& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeStatus);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::SmeDisconnect& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeDisconnect);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LoggerGetCategories& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LoggerGetCategories);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LoggerSetCategories& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LoggerSetCategories);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclGet& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclGet);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclList& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclList);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclRemove& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclRemove);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclCreate& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclCreate);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclUpdate& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclUpdate);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclLookup& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclLookup);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclRemoveAddresses& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclRemoveAddresses);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclAddAddresses& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclAddAddresses);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmAddGroup& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmAddGroup);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmDeleteGroup& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmDeleteGroup);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmAddAddr& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmAddAddr);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmCheck& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmCheck);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmDelAddr& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmDelAddr);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmAddAbonent& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmAddAbonent);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmDelAbonent& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmDelAbonent);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmListAbonents& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmListAbonents);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AliasAdd& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AliasAdd);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AliasDel& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AliasDel);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::UpdateProfileAbntResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_UpdateProfileAbntResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::GetServicesStatus& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_GetServicesStatus);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DisconnectService& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DisconnectService);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LockConfig& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LockConfig);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::UnlockConfig& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_UnlockConfig);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::RegisterAsLoadBalancer& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_RegisterAsLoadBalancer);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::RegisterAsWebapp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_RegisterAsWebapp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::RegisterAsSmsc& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_RegisterAsSmsc);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyRoutesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyRoutesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyRescheduleResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyRescheduleResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyLocaleResourceResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyLocaleResourceResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyTimeZonesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyTimeZonesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyFraudControlResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyFraudControlResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplyMapLimitsResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyMapLimitsResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ApplySnmpResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplySnmpResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::TraceRouteResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_TraceRouteResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LoadRoutesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LoadRoutesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LookupProfileResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LookupProfileResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LookupProfileExResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LookupProfileExResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::UpdateProfileResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_UpdateProfileResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DeleteProfileResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DeleteProfileResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CancelSmsResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CancelSmsResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::MscAddResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscAddResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::MscRemoveResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscRemoveResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::SmeAddResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeAddResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::SmeUpdateResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeUpdateResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::SmeRemoveResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeRemoveResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::SmeStatusResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeStatusResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::SmeDisconnectResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeDisconnectResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LoggerGetCategoriesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LoggerGetCategoriesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LoggerSetCategoriesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LoggerSetCategoriesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclGetResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclGetResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclListResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclListResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclRemoveResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclRemoveResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclCreateResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclCreateResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclUpdateResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclUpdateResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclLookupResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclLookupResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclRemoveAddressesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclRemoveAddressesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AclAddAddressesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclAddAddressesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmAddGroupResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmAddGroupResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmDeleteGroupResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmDeleteGroupResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmAddAddrResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmAddAddrResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmCheckResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmCheckResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmDelAddrResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmDelAddrResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmAddAbonentResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmAddAbonentResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmDelAbonentResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmDelAbonentResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::CgmListAbonentsResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmListAbonentsResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AliasAddResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AliasAddResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::AliasDelResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AliasDelResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::UpdateProfileAbnt& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_UpdateProfileAbnt);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::GetServicesStatusResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_GetServicesStatusResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DisconnectServiceResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DisconnectServiceResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LockConfigResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LockConfigResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}

}
}
}
