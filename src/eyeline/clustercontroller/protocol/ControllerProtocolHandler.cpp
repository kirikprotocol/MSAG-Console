#include "ControllerProtocolHandler.hpp"
#include "Magics.hpp"
#include "util/Exception.hpp"
#include "eyeline/clustercontroller/NetworkProtocol.hpp"

namespace eyeline {
namespace clustercontroller {
namespace protocol {

void ControllerProtocolHandler::handle(const messages::MultipartMessageRequest& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ReplaceIfPresentRequest& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LockProfiler& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::UnlockProfiler& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::RegisterAsLoadBalancer& msg)
{
  if(msg.getMagic()!=pmLoadBalancer)
  {
    throw smsc::util::Exception("Invalid magic for RegisterAsLoadBalancer:%x",msg.getMagic());
  }
  NetworkProtocol::getInstance()->setConnType(connId,ctLoadBalancer);
}
void ControllerProtocolHandler::handle(const messages::RegisterAsWebapp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::RegisterAsSmsc& msg)
{
  if(msg.getMagic()!=pmSmsc)
  {
    throw smsc::util::Exception("Invalid magic for RegisterAsLoadBalancer:%x",msg.getMagic());
  }
  NetworkProtocol::getInstance()->setConnType(connId,ctSmsc);
}
void ControllerProtocolHandler::handle(const messages::ApplyRoutesResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplyRescheduleResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplyLocaleResourceResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplyTimeZonesResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplyFraudControlResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplyMapLimitsResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplySnmpResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::TraceRouteResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LoadRoutesResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LookupProfileResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LookupProfileExResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::UpdateProfileResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DeleteProfileResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CancelSmsResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::MscRegisterResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::MscUnregisterResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::MscBlockResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::MscClearResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::MscListResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeAddResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeUpdateResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeRemoveResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeStatusResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeDisconnectResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LoggerGetCategoriesResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LoggerSetCategoriesResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclGetResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclListResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclRemoveResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclCreateResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclUpdateResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclLookupResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclRemoveAddressesResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclAddAddressesResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlPrcListResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlPrcAddResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlPrcDeleteResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlPrcGetResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlPrcAlterResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlMemAddResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlMemDeleteResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlMemGetResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlSbmAddResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlSbmDelResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlSbmListResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlAddResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlDeleteResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlGetResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlListResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlAlterResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlCopyResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlRenameResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmAddGroupResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmDeleteGroupResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmAddAddrResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmCheckResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmDelAddrResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmAddAbonentResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmDelAbonentResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmListAbonentsResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AliasAddResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AliasDelResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::GetServicesStatusResp& msg)
{
  printf("received GetServicesStatusResp:%s\n",msg.toString().c_str());
}
void ControllerProtocolHandler::handle(const messages::DisconnectServiceResp& msg)
{
  printf("received DisconnectServiceResp:%s\n",msg.toString().c_str());
}


}
}
}
