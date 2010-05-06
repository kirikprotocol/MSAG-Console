#include "ControllerProtocolHandler.hpp"
#include "Magics.hpp"
#include "util/Exception.hpp"
#include "eyeline/clustercontroller/NetworkProtocol.hpp"
#include "eyeline/clustercontroller/ConfigLockManager.hpp"
#include "eyeline/clustercontroller/profiler/ProfilerConfig.hpp"
#include "sms/sms.h"
#include "smsc/profiler/profiler.hpp"

namespace eyeline {
namespace clustercontroller {
namespace protocol {

void ControllerProtocolHandler::handle(const messages::ApplyRoutes& msg)
{
  //NetworkProtocol::getInstance()->enqueueCommandToType(ctSmsc,msg);
  //!!!TODO!!!
  messages::ApplyRoutes msg2=msg;
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyRoutes,messages::ApplyRoutesResp>(connId,msg2,ctRoutes);
}
void ControllerProtocolHandler::handle(const messages::ApplyReschedule& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplyLocaleResource& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplyTimeZones& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplyFraudControl& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplyMapLimits& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ApplySnmp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::TraceRoute& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LoadRoutes& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LookupProfile& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LookupProfileEx& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::UpdateProfile& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DeleteProfile& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CancelSms& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::MscAdd& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::MscRemove& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeAdd& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeUpdate& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeRemove& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeStatus& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeDisconnect& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LoggerGetCategories& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LoggerSetCategories& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclGet& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclList& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclRemove& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclCreate& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclUpdate& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclLookup& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclRemoveAddresses& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AclAddAddresses& msg)
{
  //!!!TODO!!!
}
/*
void ControllerProtocolHandler::handle(const messages::DlPrcList& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlPrcAdd& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlPrcDelete& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlPrcGet& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlPrcAlter& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlMemAdd& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlMemDelete& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlMemGet& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlSbmAdd& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlSbmDel& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlSbmList& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlAdd& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlDelete& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlGet& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlList& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlAlter& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlCopy& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DlRename& msg)
{
  //!!!TODO!!!
}
*/
void ControllerProtocolHandler::handle(const messages::CgmAddGroup& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmDeleteGroup& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmAddAddr& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmCheck& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmDelAddr& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmAddAbonent& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmDelAbonent& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::CgmListAbonents& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AliasAdd& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::AliasDel& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::GetServicesStatus& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DisconnectService& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::MultipartMessageRequest& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ReplaceIfPresentRequest& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LockConfig& msg)
{
  if(msg.getWriteLock())
  {
    ConfigLockManager::getInstance()->LockForWrite((ConfigType)msg.getConfigType(),connId);
  }else
  {
    ConfigLockManager::getInstance()->LockForRead((ConfigType)msg.getConfigType(),connId);
  }
  messages::LockConfigResp resp;
  prepareResp(msg,resp,0);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}
void ControllerProtocolHandler::handle(const messages::UnlockConfig& msg)
{
  ConfigLockManager::getInstance()->Unlock((ConfigType)msg.getConfigType(),connId);
  NetworkProtocol::getInstance()->markConfigAsLoaded(connId,(ConfigType)msg.getConfigType());

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
  NetworkProtocol::getInstance()->setConnNodeIdx(connId,msg.getNodeIndex());
}
void ControllerProtocolHandler::handle(const messages::ApplyRoutesResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
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
void ControllerProtocolHandler::handle(const messages::MscAddResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::MscRemoveResp& msg)
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
/*
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
*/
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
void ControllerProtocolHandler::handle(const messages::MultipartMessageRequestResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::ReplaceIfPresentRequestResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::LockConfigResp& msg)
{
  //!!!TODO!!!
}

void ControllerProtocolHandler::handle(const messages::UpdateProfileAbnt& msg)
{
  bool lockStatus=eyeline::clustercontroller::ConfigLockManager::getInstance()->TryLockForWrite(ctProfiles,connId);
  if(lockStatus)
  {
    smsc::sms::Address addr=msg.getAddress().c_str();
    smsc::profiler::Profile p;
    const messages::Profile& prof=msg.getProf();

    p.accessMaskIn=prof.getAccessMaskIn();
    p.accessMaskOut=prof.getAccessMaskOut();
    p.closedGroupId=prof.getClosedGroupId();
    p.codepage=prof.getCodepage();
    p.divert=prof.getDivert();
    p.divertActive=prof.getDivertActive();
    p.divertActiveAbsent=prof.getDivertActiveAbsent();
    p.divertActiveBarred=prof.getDivertActiveBarred();
    p.divertActiveBlocked=prof.getDivertActiveBlocked();
    p.divertActiveCapacity=prof.getDivertActiveCapacity();
    p.divertModifiable=prof.getDivertModifiable();
    p.hide=prof.getHide();
    p.hideModifiable=prof.getHideModifiable();
    p.locale=prof.getLocale();
    //prof.setNick(p.n)
    p.reportoptions=prof.getReportOptions();
    p.translit=prof.getTranslit();
    p.udhconcat=prof.getUdhConcat();
#ifdef SMSEXTRA
    p.sponsored=prof.getSponsored();
    p.subscription=prof.getSubscription();
    //prof.getNick(p.nick);
#endif

    eyeline::clustercontroller::profiler::ProfilerConfig::getProfiler()->update(addr,p);
    messages::UpdateProfile up;
    up.setAddress(msg.getAddress());
    up.setProf(msg.getProf());
    //up.setSeqNum(NetworkProtocol::getInstance()->);
    NetworkProtocol::getInstance()->enqueueCommandToType(ctSmsc,up,connId);
  }
  messages::UpdateProfileAbntResp resp;
  prepareResp(msg,resp,lockStatus?0:1);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
  if(lockStatus)
  {
    eyeline::clustercontroller::ConfigLockManager::getInstance()->Unlock(ctProfiles,connId);
  }
}

void ControllerProtocolHandler::handle(const messages::UpdateProfileAbntResp& msg)
{

}
/*
void ControllerProtocolHandler::handle(const messages::DlMemAddAbnt& msg)
{

}
void ControllerProtocolHandler::handle(const messages::DlMemDeleteAbnt& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlSbmAddAbnt& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlSbmDelAbnt& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlAddAbnt& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlDeleteAbnt& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlCopyAbnt& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlRenameAbnt& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlMemAddAbntResp& msg)
{

}
void ControllerProtocolHandler::handle(const messages::DlMemDeleteAbntResp& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlSbmAddAbntResp& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlSbmDelAbntResp& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlAddAbntResp& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlDeleteAbntResp& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlCopyAbntResp& msg)
{

}

void ControllerProtocolHandler::handle(const messages::DlRenameAbntResp& msg)
{

}
*/

}
}
}
