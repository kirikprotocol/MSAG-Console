#include "ControllerProtocol.hpp"
#include "eyeline/clustercontroller/protocol/messages/MultipartMessageRequest.hpp"
#include "eyeline/clustercontroller/protocol/messages/ReplaceIfPresentRequest.hpp"
#include "eyeline/clustercontroller/protocol/messages/LockProfiler.hpp"
#include "eyeline/clustercontroller/protocol/messages/UnlockProfiler.hpp"
#include "eyeline/clustercontroller/protocol/messages/RegisterAsLoadBalancer.hpp"
#include "eyeline/clustercontroller/protocol/messages/RegisterAsWebapp.hpp"
#include "eyeline/clustercontroller/protocol/messages/RegisterAsSmsc.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyRoutesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyRescheduleResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyLocaleResourceResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyTimeZonesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyFraudControlResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplyMapLimitsResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/ApplySnmpResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/TraceRouteResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LoadRoutesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LookupProfileResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LookupProfileExResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/UpdateProfileResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DeleteProfileResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CancelSmsResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscRegisterResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscUnregisterResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscBlockResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscClearResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/MscListResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeUpdateResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeRemoveResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeStatusResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/SmeDisconnectResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LoggerGetCategoriesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/LoggerSetCategoriesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclGetResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclListResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclRemoveResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclCreateResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclUpdateResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclLookupResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclRemoveAddressesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AclAddAddressesResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcListResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcDeleteResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcGetResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlPrcAlterResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlMemAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlMemDeleteResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlMemGetResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlSbmAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlSbmDelResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlSbmListResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlDeleteResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlGetResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlListResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlAlterResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlCopyResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DlRenameResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmAddGroupResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmDeleteGroupResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmAddAddrResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmCheckResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmDelAddrResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmAddAbonentResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmDelAbonentResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/CgmListAbonentsResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AliasAddResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/AliasDelResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/GetServicesStatusResp.hpp"
#include "eyeline/clustercontroller/protocol/messages/DisconnectServiceResp.hpp"

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
    case tag_MultipartMessageRequest:
    {
      messages::MultipartMessageRequest msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_ReplaceIfPresentRequest:
    {
      messages::ReplaceIfPresentRequest msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_LockProfiler:
    {
      messages::LockProfiler msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_UnlockProfiler:
    {
      messages::UnlockProfiler msg;
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
    case tag_MscRegisterResp:
    {
      messages::MscRegisterResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscUnregisterResp:
    {
      messages::MscUnregisterResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscBlockResp:
    {
      messages::MscBlockResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscClearResp:
    {
      messages::MscClearResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_MscListResp:
    {
      messages::MscListResp msg;
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
    case tag_DlPrcListResp:
    {
      messages::DlPrcListResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlPrcAddResp:
    {
      messages::DlPrcAddResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlPrcDeleteResp:
    {
      messages::DlPrcDeleteResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlPrcGetResp:
    {
      messages::DlPrcGetResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlPrcAlterResp:
    {
      messages::DlPrcAlterResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlMemAddResp:
    {
      messages::DlMemAddResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlMemDeleteResp:
    {
      messages::DlMemDeleteResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlMemGetResp:
    {
      messages::DlMemGetResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlSbmAddResp:
    {
      messages::DlSbmAddResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlSbmDelResp:
    {
      messages::DlSbmDelResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlSbmListResp:
    {
      messages::DlSbmListResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlAddResp:
    {
      messages::DlAddResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlDeleteResp:
    {
      messages::DlDeleteResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlGetResp:
    {
      messages::DlGetResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlListResp:
    {
      messages::DlListResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlAlterResp:
    {
      messages::DlAlterResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlCopyResp:
    {
      messages::DlCopyResp msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DlRenameResp:
    {
      messages::DlRenameResp msg;
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
void ControllerProtocol::encodeMessage(const messages::MscRegister& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscRegister);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::MscUnregister& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscUnregister);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::MscBlock& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscBlock);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::MscClear& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscClear);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::MscList& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MscList);
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
void ControllerProtocol::encodeMessage(const messages::DlPrcList& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlPrcList);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlPrcAdd& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlPrcAdd);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlPrcDelete& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlPrcDelete);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlPrcGet& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlPrcGet);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlPrcAlter& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlPrcAlter);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlMemAdd& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlMemAdd);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlMemDelete& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlMemDelete);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlMemGet& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlMemGet);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlSbmAdd& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlSbmAdd);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlSbmDel& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlSbmDel);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlSbmList& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlSbmList);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlAdd& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlAdd);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlDelete& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlDelete);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlGet& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlGet);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlList& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlList);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlAlter& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlAlter);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlCopy& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlCopy);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::DlRename& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DlRename);
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
void ControllerProtocol::encodeMessage(const messages::MultipartMessageRequestResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_MultipartMessageRequestResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::ReplaceIfPresentRequestResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_ReplaceIfPresentRequestResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ControllerProtocol::encodeMessage(const messages::LockProfilerResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_LockProfilerResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}

}
}
}
