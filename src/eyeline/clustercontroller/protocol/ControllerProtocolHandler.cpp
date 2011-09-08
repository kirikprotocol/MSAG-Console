#include "ControllerProtocolHandler.hpp"
#include "Magics.hpp"
#include "util/Exception.hpp"
#include "eyeline/clustercontroller/NetworkProtocol.hpp"
#include "eyeline/clustercontroller/ConfigLockManager.hpp"
#include "eyeline/clustercontroller/profiler/ProfilerConfig.hpp"
#include "sms/sms.h"
#include "smsc/profiler/profiler.hpp"
#include "eyeline/clustercontroller/router/RouterConfig.hpp"
#include "smsc/router/Router.hpp"
#include "eyeline/clustercontroller/alias/AliasConfig.hpp"
#include "eyeline/clustercontroller/acl/AclConfig.hpp"
#include "smsc/closedgroups/ClosedGroupsInterface.hpp"
#include "messages/GetSmscConfigsState.hpp"
#include "eyeline/clustercontroller/configregistry/ConfigRegistry.hpp"
#include "messages/CheckRoutesResp.hpp"


namespace eyeline {
namespace clustercontroller {
namespace protocol {

void ControllerProtocolHandler::handle(messages::ApplyRoutes& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  router::RouterConfig::Reload();
  configregistry::ConfigRegistry::getInstance()->update(ctRoutes);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyRoutes,messages::ApplyRoutesResp>(connId,msg,ctRoutes);
}
void ControllerProtocolHandler::handle(messages::ApplyReschedule& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  configregistry::ConfigRegistry::getInstance()->update(ctReschedule);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyReschedule,messages::ApplyRescheduleResp>(connId,msg,ctReschedule);
}
void ControllerProtocolHandler::handle(messages::ApplyLocaleResource& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  configregistry::ConfigRegistry::getInstance()->update(ctResources);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyLocaleResource,messages::ApplyLocaleResourceResp>(connId,msg,ctResources);
}
void ControllerProtocolHandler::handle(messages::ApplyTimeZones& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  configregistry::ConfigRegistry::getInstance()->update(ctTimeZones);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyTimeZones,messages::ApplyTimeZonesResp>(connId,msg,ctTimeZones);
}
void ControllerProtocolHandler::handle(messages::ApplyFraudControl& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  configregistry::ConfigRegistry::getInstance()->update(ctFraud);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyFraudControl,messages::ApplyFraudControlResp>(connId,msg,ctFraud);
}
void ControllerProtocolHandler::handle(messages::ApplyMapLimits& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  configregistry::ConfigRegistry::getInstance()->update(ctMapLimits);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyMapLimits,messages::ApplyMapLimitsResp>(connId,msg,ctMapLimits);
}
void ControllerProtocolHandler::handle(messages::ApplyNetProfiles& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  configregistry::ConfigRegistry::getInstance()->update(ctNetProfiles);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplyNetProfiles,messages::ApplyNetProfilesResp>(connId,msg,ctNetProfiles);
}
void ControllerProtocolHandler::handle(messages::ApplySnmp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  configregistry::ConfigRegistry::getInstance()->update(ctSnmp);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::ApplySnmp,messages::ApplySnmpResp>(connId,msg,ctSnmp);
}
void ControllerProtocolHandler::handle(const messages::TraceRoute& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::router::Router* router=eyeline::clustercontroller::router::RouterConfig::getInstance();
  messages::TraceRouteResp resp;
  //router->enableTrace(true);
  smsc::smeman::SmeProxy* proxy;
  smsc::router::RoutingResult rr;
  bool res;
  smsc::sms::Address srcAddr=msg.getSrc().c_str();
  smsc::sms::Address dstAddr=msg.getDst().c_str();

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

  smsc::smeman::SmeIndex idx=eyeline::clustercontroller::router::RouterConfig::getSmeTable()->lookup(msg.getSrcSysId().c_str());
  res=router->lookup(idx,srcAddr,dstAddr,rr);
  if(rr.found)
  {
    resp.setFound(true);
    resp.setRouteId(rr.info.routeId);
  }else
  {
    resp.setFound(false);
  }
  prepareResp(msg,resp,0);
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
  p.reportoptions=prof.getReportOptions().getValue();
  p.hide=prof.getHide().getValue();
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
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  int matchType;
  std::string matchAddr;
  smsc::profiler::Profile p=eyeline::clustercontroller::profiler::ProfilerConfig::getProfiler()->lookupEx(msg.getAddress().c_str(),matchType,matchAddr);
  messages::LookupProfileResp resp;
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
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  ConfigLockManager::getInstance()->LockForWrite(ctProfiles,connId);
  int res=0;
  try{
    smsc::profiler::Profile p;
    FillProfileFromMsg(p,msg.getProf());
    smsc::sms::Address addr=msg.getAddress().c_str();
    if(isMask(addr))
    {
      res=profiler::ProfilerConfig::getProfiler()->updatemask(addr,p);
    }else
    {
      res=profiler::ProfilerConfig::getProfiler()->update(addr,p);
    }
    if(res!=smsc::profiler::pusError)
    {
    configregistry::ConfigRegistry::getInstance()->update(ctProfiles);
    }
  }catch(std::exception& e)
  {
    res=smsc::profiler::pusError;
  }
  if(res==smsc::profiler::pusError)
  {
    messages::UpdateProfileResp resp;
    prepareMultiResp(msg,resp,1);
    NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
    return;
  }
  ConfigLockManager::getInstance()->Unlock(ctProfiles,connId);

  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::UpdateProfile,messages::UpdateProfileResp>(connId,msg,ctProfiles);
}
void ControllerProtocolHandler::handle(messages::DeleteProfile& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  ConfigLockManager::getInstance()->LockForWrite(ctProfiles,connId);
  try{
    smsc::sms::Address addr=msg.getAddress().c_str();
    profiler::ProfilerConfig::getProfiler()->remove(addr);
    configregistry::ConfigRegistry::getInstance()->update(ctProfiles);
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
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CancelSms,messages::CancelSmsResp>(connId,msg,ctMainConfig);
}
void ControllerProtocolHandler::handle(messages::MscAdd& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  configregistry::ConfigRegistry::getInstance()->update(ctMsc);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::MscAdd,messages::MscAddResp>(connId,msg,ctMsc);
}
void ControllerProtocolHandler::handle(messages::MscRemove& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  configregistry::ConfigRegistry::getInstance()->update(ctMsc);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::MscRemove,messages::MscRemoveResp>(connId,msg,ctMsc);
}

static void MsgToSmeInfo(const messages::SmeParams& sp,smsc::smeman::SmeInfo& si)
{
  using namespace smsc::smeman;
  si.systemId=sp.getId();
  si.priority=sp.getPriority();
  si.systemType=sp.getSystemType();
  si.password=sp.getPassword();
  si.rangeOfAddress=sp.getAddrRange();
  si.SME_N=sp.getSmeN();
  si.wantAlias=sp.getWantAlias();
  //si.forceDC=sp.getForceDC();
  si.timeout=sp.getTimeout();
  si.receiptSchemeName=sp.getReceiptScheme();
  si.disabled=sp.getDisabled();
  si.bindMode=(SmeBindMode)sp.getMode().getValue();
  si.proclimit=sp.getProcLimit();
  si.schedlimit=sp.getSchedLimit();
  si.accessMask=sp.getAccessMask();
  si.flags=0;
  if(sp.getFlagCarryOrgDescriptor())si.flags|=sfCarryOrgDescriptor;
  if(sp.getFlagCarryOrgAbonentInfo())si.flags|=sfCarryOrgAbonentInfo;
  if(sp.getFlagCarrySccpInfo())si.flags|=sfCarrySccpInfo;
  if(sp.getFlagFillExtraDescriptor())si.flags|=sfFillExtraDescriptor;
  if(sp.getFlagForceReceiptToSme())si.flags|=sfForceReceiptToSme;
  if(sp.getFlagForceGsmDatacoding())si.flags|=sfForceGsmDatacoding;
  if(sp.getFlagSmppPlus())si.flags|=sfSmppPlus;
}


void ControllerProtocolHandler::handle(messages::SmeAdd& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::smeman::SmeInfo si;
  MsgToSmeInfo(msg.getParams(),si);
  router::RouterConfig::getSmeAdm()->addSme(si);
  configregistry::ConfigRegistry::getInstance()->update(ctSme);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::SmeAdd,messages::SmeAddResp>(connId,msg,ctSme);
}
void ControllerProtocolHandler::handle(messages::SmeUpdate& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::smeman::SmeInfo si;
  MsgToSmeInfo(msg.getParams(),si);
  router::RouterConfig::getSmeAdm()->updateSmeInfo(si.systemId,si);
  configregistry::ConfigRegistry::getInstance()->update(ctSme);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::SmeUpdate,messages::SmeUpdateResp>(connId,msg,ctSme);
}
void ControllerProtocolHandler::handle(messages::SmeRemove& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  router::RouterConfig::getSmeAdm()->deleteSme(msg.getSmeId().c_str());
  configregistry::ConfigRegistry::getInstance()->update(ctSme);
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::SmeRemove,messages::SmeRemoveResp>(connId,msg,ctSme);
}
void ControllerProtocolHandler::handle(messages::SmeStatus& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  messages::SmeStatusResp resp;
  int seq=msg.messageGetSeqNum();
  NetworkProtocol* np=NetworkProtocol::getInstance();
  np->createGatherReq(connId,seq,resp);
  std::vector<int> ids;
  np->getConnIdsOfType(ctSmsc,ids);
  for(std::vector<int>::iterator it=ids.begin(),end=ids.end();it!=end;++it)
  {
    np->enqueueCommand(*it,msg,true);
    np->addToGatherReq(connId,seq,*it,msg.messageGetSeqNum());
  }
  ids.clear();
  np->getConnIdsOfType(ctLoadBalancer,ids);
  messages::GetServicesStatus req;
  for(std::vector<int>::iterator it=ids.begin(),end=ids.end();it!=end;++it)
  {
    np->enqueueCommand(*it,req,true);
    np->addToGatherReq(connId,seq,*it,req.messageGetSeqNum());
  }
  np->finishGatherReq(connId,seq);
}
void ControllerProtocolHandler::handle(messages::SmeDisconnect& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  messages::DisconnectService ds;
  for(std::vector<std::string>::const_iterator it=msg.getSysIds().begin(),end=msg.getSysIds().end();it!=end;++it)
  {
    ds.setServiceId(*it);
    NetworkProtocol::getInstance()->enqueueCommandToType(ctLoadBalancer,ds);
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::SmeDisconnect,messages::SmeDisconnectResp>(connId,msg,ctSme);
}
void ControllerProtocolHandler::handle(messages::LoggerGetCategories& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  if(!NetworkProtocol::getInstance()->
      enqueueCommandAnyOfType<messages::LoggerGetCategories,messages::LoggerGetCategoriesResp>(ctSmsc,connId,msg))
  {
    messages::LoggerGetCategoriesResp resp;
    prepareResp(msg,resp,1);
    NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
  }
}
void ControllerProtocolHandler::handle(messages::LoggerSetCategories& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  int orgSeqNum=msg.messageGetSeqNum();
  NetworkProtocol::getInstance()->enqueueCommandToType(ctSmsc,msg);
  messages::LoggerSetCategoriesResp resp;
  prepareResp(msg,resp,0);
  resp.messageSetSeqNum(orgSeqNum);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
}

static void MsgToAclInfo(const messages::AclInfo& msg,smsc::acl::AclInfo& acl)
{
  acl.ident=msg.getId();
  acl.name=msg.getName();
  acl.description=msg.getDescription();
}

static void AclToMsg(const smsc::acl::AclInfo& acl,messages::AclInfo& msg)
{
  msg.setId(acl.ident);
  msg.setName(acl.name);
  msg.setDescription(acl.description);
}

void ControllerProtocolHandler::handle(const messages::AclGet& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::acl::AclStore* aclMgr=acl::AclConfig::getInstance();
  messages::AclGetResp resp;
  int status=0;
  try{
    smsc::acl::AclInfo ai=aclMgr->getInfo(msg.getAclId());
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
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::acl::AclStore* aclMgr=acl::AclConfig::getInstance();
  typedef std::vector<smsc::acl::AclInfo> AclNames;
  AclNames resultList;
  int status=0;
  std::vector<messages::AclInfo> info;
  messages::AclListResp resp;
  try {
    aclMgr->enumerate(resultList);
    for(AclNames::const_iterator it=resultList.begin(),end=resultList.end();it!=end;it++)
    {
      messages::AclInfo acl;
      AclToMsg(*it,acl);
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
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::acl::AclStore* aclMgr=acl::AclConfig::getInstance();
  try{
    aclMgr->remove(msg.getAclId());
    configregistry::ConfigRegistry::getInstance()->update(ctAcl);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"failed to remove acl:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AclRemove,messages::AclRemoveResp>(connId,msg,ctAcl);
}
void ControllerProtocolHandler::handle(messages::AclCreate& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::acl::AclStore* aclMgr=acl::AclConfig::getInstance();
  messages::AclCreateResp resp;
  try{
    smsc::acl::AclIdent id=aclMgr->create2(msg.getName().c_str(),msg.getDescription().c_str(),msg.getAddresses());
    msg.setId(id);
    configregistry::ConfigRegistry::getInstance()->update(ctAcl);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"failed to create acl:%s",e.what());
    resp.messageSetSeqNum(msg.messageGetSeqNum());
    messages::MultiResponse r;
    r.getStatusRef().push_back(1);
    r.getIdsRef();
    resp.setResp(r);
    NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);
    return;
  }
  NetworkProtocol::getInstance()->enqueueMultirespCommand<messages::AclCreate,messages::AclCreateResp>(connId,msg,ctAcl);
}
void ControllerProtocolHandler::handle(messages::AclUpdate& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::acl::AclStore* aclMgr=acl::AclConfig::getInstance();
  try{
    const messages::AclInfo& ai=msg.getAcl();
    aclMgr->updateAclInfo(ai.getId(),ai.getName().c_str(),ai.getDescription().c_str());
    configregistry::ConfigRegistry::getInstance()->update(ctAcl);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"failed to update acl:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AclUpdate,messages::AclUpdateResp>(connId,msg,ctAcl);
}
void ControllerProtocolHandler::handle(const messages::AclLookup& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::acl::AclStore* aclMgr=acl::AclConfig::getInstance();
  messages::AclLookupResp resp;
  int status=0;
  typedef std::vector<smsc::acl::AclPhoneNumber> Phones;
  Phones resultPhones;

  try {
    aclMgr->lookupByPrefix(msg.getAclId(), msg.getAddrPrefix().c_str(), resultPhones);
    std::vector<std::string>  result;
    result.insert(result.begin(),resultPhones.begin(),resultPhones.end());
    resp.setResult(result);
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
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::acl::AclStore* aclMgr=acl::AclConfig::getInstance();
  try{
    const std::vector<std::string>& addrs=msg.getAddrs();
    for(std::vector<std::string>::const_iterator it=addrs.begin(),end=addrs.end();it!=end;it++)
    {
      aclMgr->removePhone(msg.getAclId(),it->c_str());
    }
    configregistry::ConfigRegistry::getInstance()->update(ctAcl);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"failed to remove addr from acl:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AclRemoveAddresses,messages::AclRemoveAddressesResp>(connId,msg,ctAcl);
}
void ControllerProtocolHandler::handle(messages::AclAddAddresses& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::acl::AclStore* aclMgr=acl::AclConfig::getInstance();
  try{
    const std::vector<std::string>& addrs=msg.getAddrs();
    for(std::vector<std::string>::const_iterator it=addrs.begin(),end=addrs.end();it!=end;it++)
    {
      aclMgr->addPhone(msg.getAclId(),it->c_str());
    }
    configregistry::ConfigRegistry::getInstance()->update(ctAcl);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"failed to add addr to acl:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AclAddAddresses,messages::AclAddAddressesResp>(connId,msg,ctAcl);
}
void ControllerProtocolHandler::handle(messages::CgmAddGroup& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  try{
    cgm->AddGroup(msg.getId(),msg.getName().c_str());
    configregistry::ConfigRegistry::getInstance()->update(ctClosedGroups);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"cgm.AddGroup failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmAddGroup,messages::CgmAddGroupResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(messages::CgmDeleteGroup& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  try{
    cgm->DeleteGroup(msg.getId());
    configregistry::ConfigRegistry::getInstance()->update(ctClosedGroups);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"cgm.DeleteGroup failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmDeleteGroup,messages::CgmDeleteGroupResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(messages::CgmAddAddr& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  try{
    cgm->AddAddrToGroup(msg.getId(),msg.getAddr().c_str());
    configregistry::ConfigRegistry::getInstance()->update(ctClosedGroups);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"cgm.AddAddr failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmAddAddr,messages::CgmAddAddrResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(const messages::CgmCheck& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
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
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  try{
    cgm->RemoveAddrFromGroup(msg.getId(),msg.getAddr().c_str());
    configregistry::ConfigRegistry::getInstance()->update(ctClosedGroups);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"cgm.DelAddr failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmDelAddr,messages::CgmDelAddrResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(messages::CgmAddAbonent& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  try{
    cgm->AddAbonent(msg.getId(),msg.getAddr().c_str());
    configregistry::ConfigRegistry::getInstance()->update(ctClosedGroups);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"cgm.AddAbonent failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::CgmAddAbonent,messages::CgmAddAbonentResp>(connId,msg,ctClosedGroups);
}
void ControllerProtocolHandler::handle(messages::CgmDelAbonent& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  int status=0;
  messages::CgmDelAbonentResp resp;
  try{
    cgm->RemoveAbonent(msg.getId(),msg.getAddr().c_str());
    configregistry::ConfigRegistry::getInstance()->update(ctClosedGroups);
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
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
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
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::alias::AliasManager* aliaser=alias::AliasConfig::getInstance();
  try{
    smsc::alias::AliasInfo ai;
    ai.addr=msg.getAddr().c_str();
    ai.alias=msg.getAlias().c_str();
    ai.hide=msg.getHide();
    aliaser->addAlias(ai);
    configregistry::ConfigRegistry::getInstance()->update(ctAliases);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"alias.add failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AliasAdd,messages::AliasAddResp>(connId,msg,ctAliases);
}
void ControllerProtocolHandler::handle(messages::AliasDel& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  smsc::alias::AliasManager* aliaser=alias::AliasConfig::getInstance();
  try{
    aliaser->deleteAlias(msg.getAlias().c_str());
    configregistry::ConfigRegistry::getInstance()->update(ctAliases);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"alias.del failed:%s",e.what());
  }
  NetworkProtocol::getInstance()->
      enqueueMultirespCommand<messages::AliasDel,messages::AliasDelResp>(connId,msg,ctAliases);
}
void ControllerProtocolHandler::handle(const messages::GetServicesStatus& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  //!!!TODO!!!
}
void ControllerProtocolHandler::handle(const messages::DisconnectService& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  //!!!TODO!!!
}

static ConfigType convertConfigType(const messages::ConfigType& configType)
{
  switch(configType.getValue())
  {
    case messages::ConfigType::MainConfig: return ctMainConfig;
    case messages::ConfigType::Profiles: return ctProfiles;
    case messages::ConfigType::Msc: return ctMsc;
    case messages::ConfigType::Routes: return ctRoutes;
    case messages::ConfigType::Sme: return ctSme;
    case messages::ConfigType::ClosedGroups: return ctClosedGroups;
    case messages::ConfigType::Aliases: return ctAliases;
    case messages::ConfigType::MapLimits: return ctMapLimits;
    case messages::ConfigType::Resources: return ctResources;
    case messages::ConfigType::Reschedule: return ctReschedule;
    case messages::ConfigType::Snmp: return ctSnmp;
    case messages::ConfigType::TimeZones: return ctTimeZones;
    case messages::ConfigType::Fraud: return ctFraud;
    case messages::ConfigType::Acl: return ctAcl;
    case messages::ConfigType::NetProfiles: return ctNetProfiles;
  }
  return ctMainConfig;
}

void ControllerProtocolHandler::handle(const messages::LockConfig& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  if(msg.getWriteLock())
  {
    ConfigLockManager::getInstance()->LockForWrite(convertConfigType(msg.getConfigType()),connId);
    if(msg.getConfigType()==messages::ConfigType::MainConfig)
    {
      configregistry::ConfigRegistry::getInstance()->update(ctMainConfig);
    }
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
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  ConfigLockManager::getInstance()->Unlock(convertConfigType(msg.getConfigType()),connId);
  NetworkProtocol::getInstance()->markConfigAsLoaded(connId,convertConfigType(msg.getConfigType()));

}
void ControllerProtocolHandler::handle(const messages::RegisterAsLoadBalancer& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  if(msg.getMagic()!=pmLoadBalancer)
  {
    throw smsc::util::Exception("Invalid magic for RegisterAsLoadBalancer:%x",msg.getMagic());
  }
  NetworkProtocol::getInstance()->setConnType(connId,ctLoadBalancer);
}
void ControllerProtocolHandler::handle(const messages::RegisterAsWebapp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  if(msg.getMagic()!=pmWebApp)
  {
    throw smsc::util::Exception("Invalid magic for RegisterAsWebapp:%x",msg.getMagic());
  }
  NetworkProtocol::getInstance()->setConnType(connId,ctWebApp);
}
void ControllerProtocolHandler::handle(const messages::RegisterAsSmsc& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  if(msg.getMagic()!=pmSmsc)
  {
    throw smsc::util::Exception("Invalid magic for RegisterAsSmsc:%x",msg.getMagic());
  }
  NetworkProtocol::getInstance()->setConnType(connId,ctSmsc);
  NetworkProtocol::getInstance()->setConnNodeIdx(connId,msg.getNodeIndex());
  const std::vector<int64_t>& v=msg.getConfigUpdateTimes();
  for(int i=0;i<ctConfigsCount;i++)
  {
    if(i==v.size())
    {
      break;
    }
    if(v[i]!=0)
    {
      NetworkProtocol::getInstance()->markConfigAsLoaded(connId,(ConfigType)i);
    }
  }
}
void ControllerProtocolHandler::handle(const messages::ApplyRoutesResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplyRescheduleResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplyLocaleResourceResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplyTimeZonesResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplyFraudControlResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplyMapLimitsResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplyNetProfilesResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::ApplySnmpResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::UpdateProfileResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::DeleteProfileResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CancelSmsResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::MscAddResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::MscRemoveResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::SmeAddResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::SmeUpdateResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::SmeRemoveResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}

struct UpdateSmeStatusOp{
  const messages::SmeStatusResp& msg;
  typedef messages::SmeStatusResp RespType;
  UpdateSmeStatusOp(const messages::SmeStatusResp& argMsg):msg(argMsg)
  {
  }
  void operator()(messages::SmeStatusResp& resp)
  {
    typedef std::vector<messages::SmeStatusInfo> SmeVector;
    SmeVector& v1=resp.getStatusRef();
    const SmeVector& v2=msg.getStatus();
    for(SmeVector::const_iterator it=v2.begin(),end=v2.end();it!=end;++it)
    {
      bool found=false;
      for(SmeVector::iterator it2=v1.begin(),end2=v1.end();it2!=end2;++it2)
      {
        if(it2->getSystemId()==it->getSystemId())
        {
          if(!it2->hasConnType())
          {
            it2->setConnType(messages::SmeConnectType::directConnect);
          }
          it2->getStatusRef().insert(it2->getStatusRef().begin(),it->getStatus().begin(),it->getStatus().end());
          found=true;
          break;
        }
      }
      if(!found)
      {
        v1.push_back(*it);
        v1.back().setConnType(messages::SmeConnectType::directConnect);
      }
    }
  }
};

void ControllerProtocolHandler::handle(const messages::SmeStatusResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol* np=NetworkProtocol::getInstance();
  UpdateSmeStatusOp op(msg);
  np->updateGatherResp(connId,msg.messageGetSeqNum(),op);
}
void ControllerProtocolHandler::handle(const messages::SmeDisconnectResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(messages::LoggerGetCategoriesResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerRespEx(connId,msg);
}
void ControllerProtocolHandler::handle(messages::LoggerSetCategoriesResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  //ignore it
}
void ControllerProtocolHandler::handle(const messages::AclRemoveResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AclCreateResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AclUpdateResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AclRemoveAddressesResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AclAddAddressesResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmAddGroupResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmDeleteGroupResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmAddAddrResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmDelAddrResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmAddAbonentResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::CgmDelAbonentResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AliasAddResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}
void ControllerProtocolHandler::handle(const messages::AliasDelResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol::getInstance()->registerMultiResp(connId,msg);
}

struct UpdateSvcStatusOp{
  const messages::GetServicesStatusResp& msg;
  typedef messages::SmeStatusResp RespType;
  UpdateSvcStatusOp(const messages::GetServicesStatusResp& argMsg):msg(argMsg)
  {
  }
  void operator()(messages::SmeStatusResp& resp)
  {
    typedef std::vector<messages::SmeStatusInfo> SmeVector;
    typedef std::vector<messages::ServiceStatus> SvcVector;
    SmeVector& v1=resp.getStatusRef();
    const SvcVector& v2=msg.getStatus();
    SmeVector::iterator it2,end2;
    for(SvcVector::const_iterator it=v2.begin(),end=v2.end();it!=end;++it)
    {
      bool found=false;
      for(it2=v1.begin(),end2=v1.end();it2!=end2;++it2)
      {
        if(it2->getSystemId()==it->getServiceName())
        {
          it2->setConnType(messages::SmeConnectType::loadBalancer);
          found=true;
          break;
        }
      }
      if(!found)
      {
        messages::SmeStatusInfo si;
        messages::SmeConnectStatus scs;
        si.setSystemId(it->getServiceName());
        scs.setBindMode(it->getBindMode());

        si.setConnType(messages::SmeConnectType::loadBalancer);
        if(!it->getPeerAddress().empty())
        {
          scs.setPeerIn(it->getPeerAddress().front());
          scs.setPeerOut(it->getPeerAddress().back());
        }
        scs.setStatus(messages::SmeStatusType::bound);

        const std::vector<int8_t>& bs=it->getBoundSmsc();
        for(std::vector<int8_t>::const_iterator bit=bs.begin(),bend=bs.end();bit!=bend;++bit)
        {
          scs.setNodeIdx(*bit);
          si.getStatusRef().push_back(scs);
        }

        v1.push_back(si);
      }else
      {
        const std::vector<int8_t>& bs=it->getBoundSmsc();
        for(std::vector<int8_t>::const_iterator bit=bs.begin(),bend=bs.end();bit!=bend;++bit)
        {
          std::vector<messages::SmeConnectStatus>& scsv=it2->getStatusRef();
          for(std::vector<messages::SmeConnectStatus>::iterator it3=scsv.begin(),end3=scsv.end();it3!=end3;++it3)
          {
            if(it3->getNodeIdx()==*bit)
            {
              if(!it->getPeerAddress().empty())
              {
                it3->setPeerIn(it->getPeerAddress().front());
                it3->setPeerOut(it->getPeerAddress().back());
              }
              it3->setStatus(messages::SmeStatusType::bound);
              break;
            }
          }
        }
      }
    }
  }
};

void ControllerProtocolHandler::handle(const messages::GetServicesStatusResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol* np=NetworkProtocol::getInstance();
  UpdateSvcStatusOp op(msg);
  np->updateGatherResp(connId,msg.messageGetSeqNum(),op);
}
void ControllerProtocolHandler::handle(const messages::DisconnectServiceResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  //ignore it
}
/*
void ControllerProtocolHandler::handle(const messages::LockConfigResp& msg)
{
  //!!!TODO!!!
}
*/

void ControllerProtocolHandler::handle(const messages::UpdateProfileAbnt& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
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
    //up.messageSetSeqNum(NetworkProtocol::getInstance()->);
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


void ControllerProtocolHandler::handle(const messages::GetConfigsState& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  messages::GetConfigsStateResp resp;
  //smsc_log_debug(log,"received GetConfigsState; seqNum=%d",msg.messageGetSeqNum());
  configregistry::ConfigRegistry::getInstance()->get(resp.getCcConfigUpdateTimeRef());
  NetworkProtocol* np=NetworkProtocol::getInstance();
  np->createGatherReq(connId,msg.messageGetSeqNum(),resp);
  std::vector<int> ids;
  np->getConnIdsOfType(ctSmsc,ids);
  messages::GetSmscConfigsState req;
  for(std::vector<int>::iterator it=ids.begin(),end=ids.end();it!=end;++it)
  {
    np->enqueueCommand(*it,req,true);
    np->addToGatherReq(connId,msg.messageGetSeqNum(),*it,req.messageGetSeqNum());
  }
  np->finishGatherReq(connId,msg.messageGetSeqNum());
}

struct UpdateSmscConfigsStateOp{
  typedef messages::GetConfigsStateResp RespType;
  const messages::GetSmscConfigsStateResp& msg;
  UpdateSmscConfigsStateOp(const messages::GetSmscConfigsStateResp& argMsg):msg(argMsg)
  {
  }
  void operator()(RespType& resp)
  {
    resp.getSmscConfigsRef().push_back(msg.getState());
  }
};

void ControllerProtocolHandler::handle(const messages::GetSmscConfigsStateResp& msg)
{
  smsc_log_debug(logDump,"%s:%s",msg.messageGetName().c_str(),msg.toString().c_str());
  NetworkProtocol* np=NetworkProtocol::getInstance();
  UpdateSmscConfigsStateOp op(msg);
  np->updateGatherResp(connId,msg.messageGetSeqNum(),op);
}

void ControllerProtocolHandler::handle(const messages::CheckRoutes& msg)
{
  messages::CheckRoutesResp resp;
  prepareResp(msg,resp,0);
  NetworkProtocol::getInstance()->enqueueCommand(connId,resp,false);

}


}
}
}
