#include "ControllerSmscProtocolClnt.hpp"
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
#include "messages/DlPrcList.hpp"
#include "messages/DlPrcAdd.hpp"
#include "messages/DlPrcDelete.hpp"
#include "messages/DlPrcGet.hpp"
#include "messages/DlPrcAlter.hpp"
#include "messages/DlMemAdd.hpp"
#include "messages/DlMemDelete.hpp"
#include "messages/DlMemGet.hpp"
#include "messages/DlSbmAdd.hpp"
#include "messages/DlSbmDel.hpp"
#include "messages/DlSbmList.hpp"
#include "messages/DlAdd.hpp"
#include "messages/DlDelete.hpp"
#include "messages/DlGet.hpp"
#include "messages/DlList.hpp"
#include "messages/DlAlter.hpp"
#include "messages/DlCopy.hpp"
#include "messages/DlRename.hpp"
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
#include "messages/GetServicesStatus.hpp"
#include "messages/DisconnectService.hpp"
#include "messages/MultipartMessageRequestResp.hpp"
#include "messages/ReplaceIfPresentRequestResp.hpp"
#include "messages/LockProfilerResp.hpp"
#include "messages/LockMscManagerResp.hpp"

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
  protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss);
}

void ControllerSmscProtocol::decodeAndHandleMessage(protogen::framework::SerializerBuffer& ss)
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
    case tag_DlPrcList:
    {
      messages::DlPrcList msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlPrcAdd:
    {
      messages::DlPrcAdd msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlPrcDelete:
    {
      messages::DlPrcDelete msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlPrcGet:
    {
      messages::DlPrcGet msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlPrcAlter:
    {
      messages::DlPrcAlter msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlMemAdd:
    {
      messages::DlMemAdd msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlMemDelete:
    {
      messages::DlMemDelete msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlMemGet:
    {
      messages::DlMemGet msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlSbmAdd:
    {
      messages::DlSbmAdd msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlSbmDel:
    {
      messages::DlSbmDel msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlSbmList:
    {
      messages::DlSbmList msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlAdd:
    {
      messages::DlAdd msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlDelete:
    {
      messages::DlDelete msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlGet:
    {
      messages::DlGet msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlList:
    {
      messages::DlList msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlAlter:
    {
      messages::DlAlter msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlCopy:
    {
      messages::DlCopy msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlRename:
    {
      messages::DlRename msg;
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
    case tag_MultipartMessageRequestResp:
    {
      messages::MultipartMessageRequestResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ReplaceIfPresentRequestResp:
    {
      messages::ReplaceIfPresentRequestResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LockProfilerResp:
    {
      messages::LockProfilerResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LockMscManagerResp:
    {
      messages::LockMscManagerResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    default:
      throw protogen::framework::UnhandledMessage(tag);
  }
}

void ControllerSmscProtocol::encodeMessage(const messages::MultipartMessageRequest& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MultipartMessageRequest);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::ReplaceIfPresentRequest& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ReplaceIfPresentRequest);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::LockProfiler& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LockProfiler);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::UnlockProfiler& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_UnlockProfiler);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::LockMscManager& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LockMscManager);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::UnlockMscManager& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_UnlockMscManager);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::RegisterAsLoadBalancer& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_RegisterAsLoadBalancer);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::RegisterAsWebapp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_RegisterAsWebapp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::RegisterAsSmsc& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_RegisterAsSmsc);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::ApplyRoutesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyRoutesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::ApplyRescheduleResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyRescheduleResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::ApplyLocaleResourceResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyLocaleResourceResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::ApplyTimeZonesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyTimeZonesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::ApplyFraudControlResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyFraudControlResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::ApplyMapLimitsResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplyMapLimitsResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::ApplySnmpResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ApplySnmpResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::TraceRouteResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_TraceRouteResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::LoadRoutesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LoadRoutesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::LookupProfileResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LookupProfileResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::LookupProfileExResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LookupProfileExResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::UpdateProfileResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_UpdateProfileResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DeleteProfileResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DeleteProfileResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::CancelSmsResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CancelSmsResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::MscAddResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscAddResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::MscRemoveResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscRemoveResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::SmeAddResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeAddResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::SmeUpdateResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeUpdateResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::SmeRemoveResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeRemoveResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::SmeStatusResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeStatusResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::SmeDisconnectResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_SmeDisconnectResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::LoggerGetCategoriesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LoggerGetCategoriesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::LoggerSetCategoriesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LoggerSetCategoriesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::AclGetResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclGetResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::AclListResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclListResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::AclRemoveResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclRemoveResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::AclCreateResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclCreateResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::AclUpdateResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclUpdateResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::AclLookupResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclLookupResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::AclRemoveAddressesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclRemoveAddressesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::AclAddAddressesResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AclAddAddressesResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlPrcListResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlPrcListResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlPrcAddResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlPrcAddResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlPrcDeleteResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlPrcDeleteResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlPrcGetResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlPrcGetResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlPrcAlterResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlPrcAlterResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlMemAddResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlMemAddResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlMemDeleteResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlMemDeleteResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlMemGetResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlMemGetResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlSbmAddResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlSbmAddResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlSbmDelResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlSbmDelResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlSbmListResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlSbmListResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlAddResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlAddResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlDeleteResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlDeleteResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlGetResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlGetResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlListResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlListResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlAlterResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlAlterResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlCopyResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlCopyResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DlRenameResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlRenameResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::CgmAddGroupResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmAddGroupResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::CgmDeleteGroupResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmDeleteGroupResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::CgmAddAddrResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmAddAddrResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::CgmCheckResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmCheckResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::CgmDelAddrResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmDelAddrResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::CgmAddAbonentResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmAddAbonentResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::CgmDelAbonentResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmDelAbonentResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::CgmListAbonentsResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_CgmListAbonentsResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::AliasAddResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AliasAddResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::AliasDelResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_AliasDelResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::GetServicesStatusResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_GetServicesStatusResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerSmscProtocol::encodeMessage(const messages::DisconnectServiceResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DisconnectServiceResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}

}
}
}
}
