#include "ControllerProtocolHandler.hpp"
#include "Magics.hpp"
#include "util/Exception.hpp"
#include "eyeline/clustercontroller/NetworkProtocol.hpp"
#include "eyeline/clustercontroller/ConfigLockManager.hpp"
#include "eyeline/clustercontroller/profiler/ProfilerConfig.hpp"
#include "sms/sms.h"
#include "smsc/profiler/profiler.hpp"
#include "eyeline/clustercontroller/router/RouterConfig.hpp"
#include "smsc/router/route_manager.h"
#include "eyeline/clustercontroller/alias/AliasConfig.hpp"
#include "eyeline/clustercontroller/acl/AclConfig.hpp"
#include "smsc/closedgroups/ClosedGroupsInterface.hpp"

namespace eyeline {
namespace clustercontroller {
namespace protocol {

void ControllerProtocolHandler::handle(messages::ApplyRoutes& msg)
{
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyRoutes,messages::ApplyRoutesResp>(connId,msg,ctRoutes);
}
void ControllerProtocolHandler::handle(messages::ApplyReschedule& msg)
{
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyReschedule,messages::ApplyRescheduleResp>(connId,msg,ctReschedule);
}
void ControllerProtocolHandler::handle(messages::ApplyLocaleResource& msg)
{
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyLocaleResource,messages::ApplyLocaleResourceResp>(connId,msg,ctResources);
}
void ControllerProtocolHandler::handle(messages::ApplyTimeZones& msg)
{
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyTimeZones,messages::ApplyTimeZonesResp>(connId,msg,ctTimeZones);
}
void ControllerProtocolHandler::handle(messages::ApplyFraudControl& msg)
{
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyFraudControl,messages::ApplyFraudControlResp>(connId,msg,ctFraud);
}
void ControllerProtocolHandler::handle(messages::ApplyMapLimits& msg)
{
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyMapLimits,messages::ApplyMapLimitsResp>(connId,msg,ctMapLimits);
}
void ControllerProtocolHandler::handle(messages::ApplySnmp& msg)
{
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplySnmp,messages::ApplySnmpResp>(connId,msg,ctSnmp);
}
void ControllerProtocolHandler::handle(const messages::TraceRoute& msg)
{
  eyeline::clustercontroller::router::RouterConfig::RouterRef router=eyeline::clustercontroller::router::RouterConfig::getInstance();
  messages::TraceRouteResp resp;
  router->enableTrace(true);
  smsc::smeman::SmeProxy* proxy;
  smsc::router::RouteInfo ri;
  bool res;
  smsc::sms::Address srcAddr=msg.getSrc().c_str();
  smsc::sms::Address dstAddr=msg.getDst().c_str();

  ri.enabling=true;

  smsc::sms::Address dealiased;
  char addrBuf[smsc::sms::MAX_ADDRESS_VALUE_LENGTH+5];
  std::string dealiasText="There are no aliases for this address";
  if(alias::AliasConfig::getInstance()->AliasToAddress(smsc::sms::Address(msg.getDst().c_str()),dealiased))
  {
    dealiasText="Address ";
    dealiasText+=smsc::sms::Address(msg.getDst().c_str()).toString().c_str();
    dealiasText+=" was dealiased to ";
    dealiasText+=dealiased.toString().c_str();
    dealiased.toString(addrBuf,sizeof(addrBuf));
    dstAddr=addrBuf;
  }
  resp.setAliasInfo(dealiasText);

  if(msg.getSrcSysId().empty())
  {
    res=router->lookup(srcAddr,dstAddr,proxy,0,&ri);
  }else
  {
    smsc::smeman::SmeIndex idx=eyeline::clustercontroller::router::RouterConfig::getSmeTable()->lookup(msg.getSrcSysId().c_str());
    res=router->lookup(idx,srcAddr,dstAddr,proxy,0,&ri);
  }
  prepareResp(msg,resp,0);
  std::vector<std::string> traceBuff;
  router->getTrace(traceBuff);
  resp.setTrace(traceBuff);
  if(res)
  {
    resp.setRouteId(ri.routeId);
    resp.setSrcAddr(ri.source.toString());
    resp.setDstAddr(ri.dest.toString());
    resp.setSmeId(ri.smeSystemId);
    resp.setSrcSmeId(ri.srcSmeSystemId);
    resp.setPriority(ri.priority);
    resp.setServiceId(ri.serviceId);
    resp.setDeliveryMode(ri.deliveryMode);
    resp.setForwardTo(ri.forwardTo);
    resp.setBilling(ri.billing);
    resp.setArchiving(ri.archived);
    resp.setEnabled(ri.enabling);
    resp.setSuppressDlvRep(ri.suppressDeliveryReports);
  }else
  {
    if(!ri.enabling)
    {
      resp.setFoundInfo(messages::RouteFindInfo::FoundDisabled);
    }else
    {
      resp.setFoundInfo(messages::RouteFindInfo::NotFound);
    }
  }
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}
void ControllerProtocolHandler::handle(const messages::LoadTestRoutes& msg)
{
  eyeline::clustercontroller::router::RouterConfig::Reload();
  messages::LoadTestRoutesResp resp;
  prepareResp(msg,resp,0);
  std::vector<std::string> trace;
  eyeline::clustercontroller::router::RouterConfig::getInstance()->getTrace(trace);
  resp.setTrace(trace);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}

static void FillMsgFromProfile(messages::Profile& prof,const smsc::profiler::Profile& p)
{
  prof.setDivert(p.divert);
  prof.setLocale(p.locale);
  prof.setCodepage(p.codepage);
  prof.setReportOptions(p.reportoptions);
  prof.setHide(p.hide);
  prof.setHideModifiable(p.hideModifiable);
  prof.setDivertActive(p.divertActive);
  prof.setDivertActiveAbsent(p.divertActiveAbsent);
  prof.setDivertActiveBlocked(p.divertActiveBlocked);
  prof.setDivertActiveBarred(p.divertActiveBarred);
  prof.setDivertActiveCapacity(p.divertActiveCapacity);
  prof.setDivertModifiable(p.divertModifiable);
  prof.setUdhConcat(p.udhconcat);
  prof.setTranslit(p.translit);
  prof.setClosedGroupId(p.closedGroupId);
  prof.setAccessMaskIn(p.accessMaskIn);
  prof.setAccessMaskOut(p.accessMaskOut);
#ifdef SMSX
  prof.setSubscription(p.subscription);
  prof.setSponsored(p.sponsored);
  prof.setNick(p.nick);
#endif
}

static void FillProfileFromMsg(smsc::profiler::Profile& p,const messages::Profile& prof)
{
  p.divert=prof.getDivert();
  p.locale=prof.getLocale();
  p.codepage=prof.getCodepage();
  p.reportoptions=prof.getReportOptions();
  p.hide=prof.getHide();
  p.hideModifiable=prof.getHideModifiable();
  p.divertActive=prof.getDivertActive();
  p.divertActiveAbsent=prof.getDivertActiveAbsent();
  p.divertActiveBlocked=prof.getDivertActiveBlocked();
  p.divertActiveBarred=prof.getDivertActiveBarred();
  p.divertActiveCapacity=prof.getDivertActiveCapacity();
  p.divertModifiable=prof.getDivertModifiable();
  p.udhconcat=prof.getUdhConcat();
  p.translit=prof.getTranslit();
  p.closedGroupId=prof.getClosedGroupId();
  p.accessMaskIn=prof.getAccessMaskIn();
  p.accessMaskOut=prof.getAccessMaskOut();
#ifdef SMSX
  p.subscription=prof.getSubscription();
  p.sponsored=prof.getSponsored();
  p.nick=prof.getNick();
#endif
}


void ControllerProtocolHandler::handle(const messages::LookupProfile& msg)
{
  smsc::profiler::Profile p=eyeline::clustercontroller::profiler::ProfilerConfig::getProfiler()->lookup(msg.getAddress().c_str());
  messages::LookupProfileResp resp;
  prepareResp(msg,resp,0);
  FillMsgFromProfile(resp.getProfRef(),p);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}
void ControllerProtocolHandler::handle(const messages::LookupProfileEx& msg)
{
  int matchType;
  std::string matchAddr;
  smsc::profiler::Profile p=eyeline::clustercontroller::profiler::ProfilerConfig::getProfiler()->lookupEx(msg.getAddress().c_str(),matchType,matchAddr);
  messages::LookupProfileExResp resp;
  prepareResp(msg,resp,0);
  FillMsgFromProfile(resp.getProfRef(),p);
  switch(matchType)
  {
    case smsc::profiler::ProfilerMatchType::mtDefault:resp.setMatchType(messages::ProfileMatchType::Default);break;
    case smsc::profiler::ProfilerMatchType::mtExact:resp.setMatchType(messages::ProfileMatchType::Exact);break;
    case smsc::profiler::ProfilerMatchType::mtMask:resp.setMatchType(messages::ProfileMatchType::Mask);break;
  }
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}

static bool isMask(const smsc::sms::Address & address)
{
  return address.length>0 && address.value[address.length-1]=='?';
}

void ControllerProtocolHandler::handle(messages::UpdateProfile& msg)
{
  ConfigLockManager::getInstance()->LockForWrite(ctProfiles,connId);
  try{
    smsc::profiler::Profile p;
    FillProfileFromMsg(p,msg.getProf());
    smsc::sms::Address addr=msg.getAddress().c_str();
    if(isMask(addr))
    {
      profiler::ProfilerConfig::getProfiler()->updatemask(addr,p);
    }else
    {
      profiler::ProfilerConfig::getProfiler()->update(addr,p);
    }
  }catch(std::exception& e)
  {
    //!!TODO!!
  }
  ConfigLockManager::getInstance()->Unlock(ctProfiles,connId);

  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::UpdateProfile,messages::UpdateProfileResp>(connId,msg,ctProfiles);
}
void ControllerProtocolHandler::handle(messages::DeleteProfile& msg)
{
  ConfigLockManager::getInstance()->LockForWrite(ctProfiles,connId);
  try{
    smsc::sms::Address addr=msg.getAddress().c_str();
    profiler::ProfilerConfig::getProfiler()->remove(addr);
  }catch(std::exception& e)
  {
    //!!TODO!!
  }
  ConfigLockManager::getInstance()->Unlock(ctProfiles,connId);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::DeleteProfile,messages::DeleteProfileResp>(connId,msg,ctProfiles);
}
void ControllerProtocolHandler::handle(messages::CancelSms& msg)
{
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CancelSms,messages::CancelSmsResp>(connId,msg,ctMainConfig);
}
void ControllerProtocolHandler::handle(messages::MscAdd& msg)
{
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::MscAdd,messages::MscAddResp>(connId,msg,ctMsc);
}
void ControllerProtocolHandler::handle(messages::MscRemove& msg)
{
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::MscRemove,messages::MscRemoveResp>(connId,msg,ctMsc);
}

static void MsgToSmeInfo(const messages::SmeParams& sp,smsc::smeman::SmeInfo& si)
{
  si.systemId=sp.getId();
  si.priority=sp.getPriority();
  si.typeOfNumber=sp.getTypeOfNumber();
  si.numberingPlan=sp.getNumberingPlan();
  si.interfaceVersion=sp.getInterfaceVersion();
  si.systemType=sp.getSystemType();
  si.password=sp.getPassword();
  si.rangeOfAddress=sp.getAddrRange();
  si.SME_N=sp.getSmeN();
  si.wantAlias=sp.getWantAlias();
  //si.forceDC=sp.getForceDC();
  si.timeout=sp.getTimeout();
  si.receiptSchemeName=sp.getReceiptScheme();
  si.disabled=sp.getDisabled();
  si.bindMode=sp.getMode()=="trx"?smsc::smeman::smeTRX:sp.getMode()=="tx"?smsc::smeman::smeTX:smsc::smeman::smeRX;
  si.proclimit=sp.getProcLimit();
  si.schedlimit=sp.getSchedLimit();
  si.accessMask=sp.getAccessMask();
  si.flags=sp.getFlags();
}


void ControllerProtocolHandler::handle(messages::SmeAdd& msg)
{
  smsc::smeman::SmeInfo si;
  MsgToSmeInfo(msg.getParams(),si);
  router::RouterConfig::getSmeAdm()->addSme(si);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::SmeAdd,messages::SmeAddResp>(connId,msg,ctSme);
}
void ControllerProtocolHandler::handle(messages::SmeUpdate& msg)
{
  smsc::smeman::SmeInfo si;
  MsgToSmeInfo(msg.getParams(),si);
  router::RouterConfig::getSmeAdm()->updateSmeInfo(si.systemId,si);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::SmeUpdate,messages::SmeUpdateResp>(connId,msg,ctSme);
}
void ControllerProtocolHandler::handle(messages::SmeRemove& msg)
{
  router::RouterConfig::getSmeAdm()->deleteSme(msg.getSmeId().c_str());
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::SmeRemove,messages::SmeRemoveResp>(connId,msg,ctSme);
}
void ControllerProtocolHandler::handle(const messages::SmeStatus& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeDisconnect& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(messages::LoggerGetCategories& msg)
{
  NetworkProtocol::getInstance()->
      enqueueCommandAnyOfType<messages::LoggerGetCategories,messages::LoggerGetCategoriesResp>(ctSmsc,connId,msg);
}
void ControllerProtocolHandler::handle(messages::LoggerSetCategories& msg)
{
  NetworkProtocol::getInstance()->enqueueCommandToType(ctSmsc,msg);
  messages::LoggerSetCategoriesResp resp;
  prepareResp(msg,resp,0);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}

static void MsgToAclInfo(const messages::AclInfo& msg,smsc::acls::AclInfo& acl)
{
  acl.ident=msg.getId();
  acl.name=msg.getName();
  acl.description=msg.getDescription();
  acl.cache=(smsc::acls::AclCacheType)msg.getCacheType();
}

static void AclToMsg(const smsc::acls::AclInfo& acl,messages::AclInfo& msg)
{
  msg.setId(acl.ident);
  msg.setName(acl.name);
  msg.setDescription(acl.description);
  msg.setCacheType(acl.cache);
}

void ControllerProtocolHandler::handle(const messages::AclGet& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=acl::AclConfig::getInstance();
  messages::AclGetResp resp;
  int status=0;
  try{
    smsc::acls::AclInfo ai=aclMgr->getInfo(msg.getAclId());
    messages::AclInfo acl;
    AclToMsg(ai,acl);
    resp.setAcl(acl);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"failed to get acl:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}
void ControllerProtocolHandler::handle(const messages::AclList& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=acl::AclConfig::getInstance();
  typedef std::vector<smsc::acls::AclNamedIdent> AclNames;
  AclNames resultList;
  int status=0;
  std::vector<messages::AclInfo> info;
  messages::AclListResp resp;
  try {
    aclMgr->enumerate(resultList);
    for(AclNames::const_iterator it=resultList.begin(),end=resultList.end();it!=end;it++)
    {
      smsc::acls::AclInfo ai=aclMgr->getInfo(it->first);
      messages::AclInfo acl;
      AclToMsg(ai,acl);
      info.push_back(acl);
    }
    resp.setAclList(info);
  } catch (std::exception &e)
  {
    smsc_log_warn(log,"failed to list acls:%s",e.what());
    status=1;
  }
  prepareResp(msg,resp,status);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}
void ControllerProtocolHandler::handle(messages::AclRemove& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=acl::AclConfig::getInstance();
  try{
    aclMgr->remove(msg.getAclId());
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"failed to remove acl:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AclRemove,messages::AclRemoveResp>(connId,msg,ctAcl);
}
void ControllerProtocolHandler::handle(messages::AclCreate& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=acl::AclConfig::getInstance();
  messages::AclCreateResp resp;
  try{
    resp.setId(aclMgr->create2(msg.getName().c_str(),msg.getDescription().c_str(),msg.getAddresses(),(smsc::acls::AclCacheType)msg.getCacheType()));
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"failed to create acl:%s",e.what());
    resp.setSeqNum(msg.getSeqNum());
    messages::MultiResponse r;
    r.getStatusRef().push_back(1);
    resp.setResp(r);
    NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
    return;
  }
  NetworkProtocol::getInstance()->enqueueMultirespCommandEx(connId,msg,resp,ctAcl);
}
void ControllerProtocolHandler::handle(messages::AclUpdate& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=acl::AclConfig::getInstance();
  try{
    const messages::AclInfo& ai=msg.getAcl();
    aclMgr->updateAclInfo(ai.getId(),ai.getName().c_str(),ai.getDescription().c_str(),(smsc::acls::AclCacheType)ai.getCacheType());
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"failed to update acl:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AclUpdate,messages::AclUpdateResp>(connId,msg,ctAcl);
}
void ControllerProtocolHandler::handle(const messages::AclLookup& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=acl::AclConfig::getInstance();
  messages::AclLookupResp resp;
  int status=0;
  typedef std::vector<smsc::acls::AclPhoneNumber> Phones;
  Phones resultPhones;

  try {
    aclMgr->lookupByPrefix(msg.getAclId(), msg.getAddrPrefix().c_str(), resultPhones);
    resp.setResult(resultPhones);
  } catch (std::exception &e)
  {
    status=1;
    smsc_log_warn(log,"failed to lookup acl:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}
void ControllerProtocolHandler::handle(messages::AclRemoveAddresses& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=acl::AclConfig::getInstance();
  try{
    const std::vector<std::string>& addrs=msg.getAddrs();
    for(std::vector<std::string>::const_iterator it=addrs.begin(),end=addrs.end();it!=end;it++)
    {
      aclMgr->removePhone(msg.getAclId(),*it);
    }
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"failed to remove addr from acl:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AclRemoveAddresses,messages::AclRemoveAddressesResp>(connId,msg,ctAcl);
}
void ControllerProtocolHandler::handle(messages::AclAddAddresses& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=acl::AclConfig::getInstance();
  try{
    const std::vector<std::string>& addrs=msg.getAddrs();
    for(std::vector<std::string>::const_iterator it=addrs.begin(),end=addrs.end();it!=end;it++)
    {
      aclMgr->addPhone(msg.getAclId(),*it);
    }
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"failed to add addr to acl:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AclAddAddresses,messages::AclAddAddressesResp>(connId,msg,ctAcl);
}
void ControllerProtocolHandler::handle(messages::CgmAddGroup& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  try{
    cgm->AddGroup(msg.getId(),msg.getName().c_str());
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"cgm.AddGroup failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmAddGroup,messages::CgmAddGroupResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(messages::CgmDeleteGroup& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  try{
    cgm->DeleteGroup(msg.getId());
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"cgm.DeleteGroup failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmDeleteGroup,messages::CgmDeleteGroupResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(messages::CgmAddAddr& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  try{
    cgm->AddAddrToGroup(msg.getId(),msg.getAddr().c_str());
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"cgm.AddAddr failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmAddAddr,messages::CgmAddAddrResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(const messages::CgmCheck& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  int status=0;
  messages::CgmCheckResp resp;
  try{
    resp.setResult(cgm->Check(msg.getId(),msg.getAddr().c_str()));
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"cgm.Check failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp);
}
void ControllerProtocolHandler::handle(messages::CgmDelAddr& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  try{
    cgm->RemoveAddrFromGroup(msg.getId(),msg.getAddr().c_str());
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"cgm.DelAddr failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmDelAddr,messages::CgmDelAddrResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(messages::CgmAddAbonent& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  try{
    cgm->AddAbonent(msg.getId(),msg.getAddr().c_str());
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"cgm.AddAbonent failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmAddAbonent,messages::CgmAddAbonentResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(messages::CgmDelAbonent& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  int status=0;
  messages::CgmDelAbonentResp resp;
  try{
    cgm->RemoveAbonent(msg.getId(),msg.getAddr().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"cgm.DelAbonent failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmDelAbonent,messages::CgmDelAbonentResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(const messages::CgmListAbonents& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  int status=0;
  messages::CgmListAbonentsResp resp;
  try{
    std::vector<std::string> result;
    std::vector<smsc::sms::Address> list;
    cgm->ListAbonents(msg.getId(),list);
    for(std::vector<smsc::sms::Address>::iterator it=list.begin();it!=list.end();it++)
    {
      result.push_back(it->toString());
    }
    resp.setResult(result);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"cgm.ListAbonents failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp);
}
void ControllerProtocolHandler::handle(messages::AliasAdd& msg)
{
  smsc::alias::AliasManager* aliaser=alias::AliasConfig::getInstance();
  try{
    smsc::alias::AliasInfo ai;
    ai.addr=msg.getAddr().c_str();
    ai.alias=msg.getAlias().c_str();
    ai.hide=msg.getHide();
    aliaser->addAlias(ai);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"alias.add failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AliasAdd,messages::AliasAddResp>(connId,msg,ctAliases);
}
void ControllerProtocolHandler::handle(messages::AliasDel& msg)
{
  smsc::alias::AliasManager* aliaser=alias::AliasConfig::getInstance();
  try{
    aliaser->deleteAlias(msg.getAlias().c_str());
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"alias.del failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AliasDel,messages::AliasDelResp>(connId,msg,ctAliases);
}
void ControllerProtocolHandler::handle(const messages::GetServicesStatus& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DisconnectService& msg)
{
  //!!!TODO!!!
}

static ConfigType convertConfigType(messages::ConfigType::type configType)
{
    if(configType==messages::ConfigType::MainConfig)return ctMainConfig;
    if(configType==messages::ConfigType::Profiles)return ctProfiles;
    if(configType==messages::ConfigType::Msc)return ctMsc;
    if(configType==messages::ConfigType::Routes)return ctRoutes;
    if(configType==messages::ConfigType::Sme)return ctSme;
    if(configType==messages::ConfigType::ClosedGroups)return ctClosedGroups;
    if(configType==messages::ConfigType::Aliases)return ctAliases;
    if(configType==messages::ConfigType::MapLimits)return ctMapLimits;
    if(configType==messages::ConfigType::Regions)return ctRegions;
    if(configType==messages::ConfigType::Resources)return ctResources;
    if(configType==messages::ConfigType::Reschedule)return ctReschedule;
    if(configType==messages::ConfigType::Snmp)return ctSnmp;
    if(configType==messages::ConfigType::TimeZones)return ctTimeZones;
    if(configType==messages::ConfigType::Fraud)return ctFraud;
    if(configType==messages::ConfigType::Acl)return ctAcl;
    return ctMainConfig;
}

void ControllerProtocolHandler::handle(const messages::LockConfig& msg)
{
  if(msg.getWriteLock())
  {
    ConfigLockManager::getInstance()->LockForWrite(convertConfigType(msg.getConfigType()),connId);
  }else
  {
    ConfigLockManager::getInstance()->LockForRead(convertConfigType(msg.getConfigType()),connId);
  }
  messages::LockConfigResp resp;
  prepareResp(msg,resp,0);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}
void ControllerProtocolHandler::handle(const messages::UnlockConfig& msg)
{
  ConfigLockManager::getInstance()->Unlock(convertConfigType(msg.getConfigType()),connId);
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
  if(msg.getMagic()!=pmWebApp)
  {
    throw smsc::util::Exception("Invalid magic for RegisterAsLoadBalancer:%x",msg.getMagic());
  }
  NetworkProtocol::getInstance()->setConnType(connId,ctWebApp);
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
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplyLocaleResourceResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplyTimeZonesResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplyFraudControlResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplyMapLimitsResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplySnmpResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::UpdateProfileResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::DeleteProfileResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CancelSmsResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::MscAddResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::MscRemoveResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::SmeAddResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::SmeUpdateResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::SmeRemoveResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::SmeStatusResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::SmeDisconnectResp& msg)
{
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(messages::LoggerGetCategoriesResp& msg)
{
  NetworkProtocol::getInstance()->registerResp(connId,msg);
}
void ControllerProtocolHandler::handle(messages::LoggerSetCategoriesResp& msg)
{
  //ignore it
}
void ControllerProtocolHandler::handle(const messages::AclRemoveResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AclCreateResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AclUpdateResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AclRemoveAddressesResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AclAddAddressesResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmAddGroupResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmDeleteGroupResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmAddAddrResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmDelAddrResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmAddAbonentResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmDelAbonentResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AliasAddResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AliasDelResp& msg)
{
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::GetServicesStatusResp& msg)
{
  printf("received GetServicesStatusResp:%s\n",msg.toString().c_str());
}
void ControllerProtocolHandler::handle(const messages::DisconnectServiceResp& msg)
{
  printf("received DisconnectServiceResp:%s\n",msg.toString().c_str());
}
/*
void ControllerProtocolHandler::handle(const messages::LockConfigResp& msg)
{
  //!!!TODO!!!
}
*/

void ControllerProtocolHandler::handle(const messages::UpdateProfileAbnt& msg)
{
  bool lockStatus=eyeline::clustercontroller::ConfigLockManager::getInstance()->TryLockForWrite(ctProfiles,connId);
  if(lockStatus)
  {
    smsc::sms::Address addr=msg.getAddress().c_str();
    smsc::profiler::Profile p;
    const messages::Profile& prof=msg.getProf();

    FillProfileFromMsg(p,prof);

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


}
}
}
