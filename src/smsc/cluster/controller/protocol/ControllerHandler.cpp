#include <memory>

#include "smsc/closedgroups/ClosedGroupsInterface.hpp"
#include "core/buffers/Array.hpp"
#include "smsc/smsc.hpp"
#include "smsc/cluster/controller/NetworkDispatcher.hpp"
#include "smsc/resourcemanager/ResourceManager.hpp"
#include "smsc/config/route/RouteConfig.h"
#include "smsc/smeman/smeproxy.h"
#include "smsc/router/route_types.h"
#include "smsc/profiler/profiler-types.hpp"
#include "smsc/common/TimeZoneMan.hpp"
#include "smsc/mapio/FraudControl.hpp"
#include "smsc/mapio/MapLimits.hpp"
#include "smsc/mscman/MscManager.h"
#include "smsc/alias/AliasMan.hpp"

#include "ControllerHandler.hpp"
#include "smsc/smeman/smsccmd.h"

using smsc::config::route::RouteConfig;
using smsc::smeman::SmeProxy;
using smsc::router::RouteInfo;
using smsc::mscman::MscManager;
using smsc::alias::AliasManager;

namespace smsc {
namespace cluster {
namespace controller {
namespace protocol {

using smsc::Smsc;

void ControllerHandler::Init()
{
  nodeIdx=NetworkDispatcher::getInstance().getNodeIndex();
}

void ControllerHandler::handle(const messages::ApplyRoutes& msg)
{
  Smsc& smsc=Smsc::getInstance();
  smsc.getConfigs()->routesconfig->reload();
  smsc.getConfigs()->smemanconfig->reload();
  smsc.reloadRoutes();
  messages::ApplyRoutesResp resp;
  prepareMultiResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::ApplyReschedule& msg)
{
  Smsc& smsc=Smsc::getInstance();
  int status=0;
  try{
    smsc.reloadReschedule();
  }catch(std::exception& e)
  {
    smsc_log_info(log,"reload reschedule failed:%s",e.what());
    status=1;
  }
  messages::ApplyRescheduleResp resp;
  prepareMultiResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::ApplyLocaleResource& msg)
{
  Smsc& smsc=Smsc::getInstance();
  smsc::util::config::Manager& cfgman=*smsc.getConfigs()->cfgman;
  smsc::resourcemanager::ResourceManager::reload(cfgman.getString("core.locales"), cfgman.getString("core.default_locale"));
  messages::ApplyLocaleResourceResp resp;
  prepareMultiResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::ApplyTimeZones& msg)
{
  int status=0;
  try{
    smsc::common::TimeZoneManager::getInstance().Reload();
  }catch(std::exception& e)
  {
    smsc_log_info(log,"apply time zones failed:%s",e.what());
    status=1;
  }
  messages::ApplyTimeZonesResp resp;
  prepareMultiResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::ApplyFraudControl& msg)
{
  int status=0;
  try{
    smsc::mapio::FraudControl::getInstance()->Reload();
  }catch(std::exception& e)
  {
    smsc_log_info(log,"apply fraud control failed:%s",e.what());
    status=1;
  }
  messages::ApplyFraudControlResp resp;
  prepareMultiResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::ApplyMapLimits& msg)
{
  int status=0;
  try{
    smsc::mapio::MapLimits::getInstance().Reinit();
  }catch(std::exception& e)
  {
    smsc_log_info(log,"apply map limits failed:%s",e.what());
    status=1;
  }
  messages::ApplyMapLimitsResp resp;
  prepareMultiResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::ApplySnmp& msg)
{
  int status=0;
  try{
#ifdef SNMP
    smsc::snmp::SnmpCounter::getInstance().ReloadCfg();
#endif
  }catch(std::exception& e)
  {
    smsc_log_info(log,"apply snmp failed:%s",e.what());
    status=1;
  }
  messages::ApplySnmpResp resp;
  prepareMultiResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
/*
void ControllerHandler::handle(const messages::TraceRoute& msg)
{
  using smsc::config::route::RouteConfig;
  RouteConfig cfg;
  int status=0;
  messages::TraceRouteResp resp;
  try{
    if (!isTempRouterLoaded && cfg.load("conf/routes__.xml") == RouteConfig::fail)
    {
      throw smsc::util::Exception("failed to load temp router config");
    }else
    {
      Smsc::getInstance().reloadTestRoutes(cfg);
      isTempRouterLoaded=true;
    }
    messages::string_list result;
    smsc::smeman::SmeProxy* proxy = 0;
    smsc::router::RouteInfo info;
    bool found = false;
    info.enabling = true;

    //-1:   Dealiased destination
    // 0:   Message (Route found | Route found (disabled) | Route not found)
    // 1:   RouteInfo (if any)
    // 2..: Trace (if any)

    smsc::sms::Address orgAddr=msg.getSrc().c_str();
    smsc::sms::Address dstAddr=msg.getDst().c_str();

    smsc::sms::Address dealiased;
    char addrBuf[smsc::sms::MAX_ADDRESS_VALUE_LENGTH+5];
    std::string dealiasText="There are no aliases for this address";
    if(Smsc::getInstance().AliasToAddress(smsc::sms::Address(msg.getDst().c_str()),dealiased))
    {
      dealiasText="Address ";
      dealiasText+=smsc::sms::Address(msg.getDst().c_str()).toString().c_str();
      dealiasText+=" was dealiased to ";
      dealiasText+=dealiased.toString().c_str();
      dealiased.toString(addrBuf,sizeof(addrBuf));
      dstAddr=addrBuf;
    }
    resp.setAliasInfo(dealiasText);

    if (msg.getSrcSysId().length())
    {
      smsc::smeman::SmeIndex index = Smsc::getInstance().getSmeIndex(msg.getSrcSysId().c_str());
      if (index == -1)
      {
        throw smsc::util::Exception("TraceRoute:Sme with id '%s' not found",msg.getSrcSysId().c_str());
      }
      found = Smsc::getInstance().getTestRouterInstance()->
          lookup(index, orgAddr, dstAddr, proxy, 0, &info);
    }
    else
    {
      found = Smsc::getInstance().getTestRouterInstance()->
          lookup(orgAddr, dstAddr, proxy, 0, &info);
    }

    vector<std::string> traceBuff;
    Smsc::getInstance().getTestRouterInstance()->getTrace(traceBuff);


    if (!found)
    {
      if (info.enabling == false)
      {
        resp.setFoundInfo("Route found (disabled)");
        found = true;
      } else
      {
        resp.setFoundInfo("Route not found");
      }
    } else
    {
      resp.setFoundInfo("Route found");
    }

    if (found)
    {
      char routeText[2048];
      char srcAddressText[64]; char dstAddressText[64];
      info.source.getText(srcAddressText, sizeof(srcAddressText));
      info.dest  .getText(dstAddressText, sizeof(dstAddressText));

      resp.setRouteId(info.routeId);
      resp.setSrcAddr(srcAddressText);
      resp.setDstAddr(dstAddressText);
      resp.setSmeId(info.smeSystemId);
      resp.setSrcSmeId(info.srcSmeSystemId);
      resp.setPriority(info.priority);
      resp.setServiceId(info.serviceId);
      resp.setDeliveryMode(info.deliveryMode);
      resp.setForwardTo(info.forwardTo);
      resp.setBilling(info.billing);
      resp.setArchiving(info.archived);
      resp.setEnabled(info.enabling);
      resp.setSuppressDlvRep(info.suppressDeliveryReports);
    }

    resp.setTrace(traceBuff);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_info(log,"trace route failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::LoadRoutes& msg)
{
  isTempRouterLoaded = false;
  messages::LoadRoutesResp resp;
  int status=0;

  try{
    smsc::config::route::RouteConfig cfg;
    if (cfg.load("conf/routes__.xml") == RouteConfig::fail)
    {
      throw smsc::util::Exception("failed to load routes config");
    }

    vector<std::string> traceBuff;
    Smsc::getInstance().reloadTestRoutes(cfg);
    Smsc::getInstance().getTestRouterInstance()->enableTrace(true);
    Smsc::getInstance().getTestRouterInstance()->getTrace(traceBuff);

    resp.setTrace(traceBuff);

    isTempRouterLoaded = true;
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_info(log,"load routes for tracing failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}

void ControllerHandler::handle(const messages::LookupProfile& msg)
{
  smsc::profiler::ProfilerInterface* profiler=Smsc::getInstance().getProfiler();
  smsc::sms::Address address(msg.getAddress().c_str());
  int matchType = smsc::profiler::ProfilerMatchType::mtExact;
  std::string matchAddress;
  smsc::profiler::Profile p=profiler->lookupEx(address, matchType, matchAddress);
  messages::Profile prof;
  messages::LookupProfileResp resp;
  FillMsgFromProfile(prof,p);
  resp.setProf(prof);
  prepareResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::LookupProfileEx& msg)
{
  smsc::profiler::ProfilerInterface* profiler=Smsc::getInstance().getProfiler();
  smsc::sms::Address address(msg.getAddress().c_str());
  int matchType = smsc::profiler::ProfilerMatchType::mtExact;
  std::string matchAddress;
  smsc::profiler::Profile p=profiler->lookupEx(address, matchType, matchAddress);
  messages::Profile prof;
  messages::LookupProfileExResp resp;
  FillMsgFromProfile(prof,p);
  resp.setProf(prof);
  resp.setMatchType(matchType);
  prepareResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
*/

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


static bool isMask(const smsc::sms::Address & address)
{
  for (unsigned i=0; i<address.length; i++)
  {
    if (address.value[i] == '?')
      return true;
  }
  return false;
}

void ControllerHandler::handle(const messages::UpdateProfile& msg)
{
  smsc::profiler::ProfilerInterface* profiler=Smsc::getInstance().getProfiler();
  smsc::sms::Address address(msg.getAddress().c_str());
  smsc::profiler::Profile p;
  FillProfileFromMsg(p,msg.getProf());
  int result;
  if(isMask(address))
  {
    result=profiler->updatemask(address,p);
  }else
  {
    result=profiler->update(address,p);
  }
  messages::UpdateProfileResp resp;
  resp.setResult(result);

  prepareResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DeleteProfile& msg)
{
  smsc::profiler::ProfilerInterface* profiler=Smsc::getInstance().getProfiler();
  smsc::sms::Address address(msg.getAddress().c_str());
  profiler->remove(address);
  messages::DeleteProfileResp resp;
  prepareResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::CancelSms& msg)
{
  const std::vector<std::string>& ids=msg.getIds();
  const std::vector<std::string>& srcs=msg.getSrcs();
  const std::vector<std::string>& dsts=msg.getDsts();
  size_t sz=ids.size();
  if(sz!=srcs.size() || sz!=dsts.size())
  {
    smsc_log_warn(log,"CancelSms: data sizes mismatch");
    messages::CancelSmsResp resp;
    prepareResp(msg,resp,1);
    NetworkDispatcher::getInstance().enqueueMessage(resp);
    return;
  }
  Smsc& smsc=Smsc::getInstance();
  for(size_t i=0;i<sz;i++)
  {
    try{
      smsc::sms::SMSId id;
      sscanf(ids[i].c_str(),"%llx",&id);
      smsc.cancelSms(id,srcs[i].c_str(),dsts[i].c_str());
    }catch(std::exception& e)
    {
      smsc_log_warn(log,"failed to cancel sms:%s",e.what());
    }
  }
  messages::CancelSmsResp resp;
  prepareResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::MscAdd& msg)
{
  smsc::mscman::MscManager::getInstance().add(msg.getMsc().c_str());
  messages::MscAddResp resp;
  prepareResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::MscRemove& msg)
{
  smsc::mscman::MscManager::getInstance().remove(msg.getMsc().c_str());
  messages::MscRemoveResp resp;
  prepareResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
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

void ControllerHandler::handle(const messages::SmeAdd& msg)
{
  smsc::smeman::SmeInfo si;
  MsgToSmeInfo(msg.getParams(),si);
  int status=0;
  try{
    Smsc::getInstance().getSmeAdmin()->addSme(si);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"SmeAdd: failed: %s",e.what());
  }
  messages::SmeAddResp resp;
  prepareMultiResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::SmeUpdate& msg)
{
  smsc::smeman::SmeInfo si;
  MsgToSmeInfo(msg.getParams(),si);
  int status=0;
  try{
    Smsc::getInstance().getSmeAdmin()->updateSmeInfo(si.systemId,si);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"SmeUpdate: failed: %s",e.what());
  }
  messages::SmeUpdateResp resp;
  prepareMultiResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::SmeRemove& msg)
{
  int status=0;
  try{
    Smsc::getInstance().getSmeAdmin()->deleteSme(msg.getSmeId().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"SmeDelete: failed: %s",e.what());
  }
  messages::SmeRemoveResp resp;
  prepareMultiResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::SmeStatus& msg)
{
  std::vector<messages::SmeStatusInfo> respList;
  for (std::auto_ptr<smsc::smeman::SmeIterator> i(Smsc::getInstance().getSmeAdmin()->iterator()); i.get() != NULL;)
  {
    messages::SmeStatusInfo ssi;
    ssi.setSystemId(i->getSmeInfo().systemId);
    if(i->getSmeInfo().internal)
    {
      ssi.setStatus("internal");
    }else
    {
      if (!i->isSmeConnected())
      {
        ssi.setStatus("disconnected");
      }else
      {
        try{
          SmeProxy * smeProxy = i->getSmeProxy();
          switch(smeProxy->getBindMode())
          {
            case smsc::smeman::smeTRX:ssi.setBindMode(messages::SmeBindMode::modeTrx);break;
            case smsc::smeman::smeTX:ssi.setBindMode(messages::SmeBindMode::modeTx);break;
            case smsc::smeman::smeRX:ssi.setBindMode(messages::SmeBindMode::modeRx);break;
            default:ssi.setBindMode(messages::SmeBindMode::modeUnknown);break;
          }
          char inIP[128], outIP[128];
          if (smeProxy->getPeers(inIP,outIP))
          {
            ssi.setPeerIn(inIP);
            ssi.setPeerOut(outIP);
          }else
          {
            ssi.setPeerIn("unknown");
            ssi.setPeerOut("unknown");
          }
        }catch(std::exception& e)
        {
          ssi.setBindMode(messages::SmeBindMode::modeUnknown);
          ssi.setPeerIn("unknown");
          ssi.setPeerOut("unknown");
        }
      }
    }
    respList.push_back(ssi);
    if (!i->next())
    {
      break;
    }
  }
  messages::SmeStatusResp resp;
  resp.setStatus(respList);
  prepareResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::SmeDisconnect& msg)
{
  const std::vector<std::string>& ids=msg.getSysIds();
  int status=0;
  try{
    for(std::vector<std::string>::const_iterator it=ids.begin(),end=ids.end();it!=end;it++)
    {
      for (std::auto_ptr<smsc::smeman::SmeIterator> i(Smsc::getInstance().getSmeAdmin()->iterator()); i.get() != NULL;)
      {
        if (i->getSmeProxy() != 0 && i->getSmeInfo().systemId == *it)
        {
          i->getSmeProxy()->disconnect();
          break;
        }
        if (!i->next())
        {
          break;
        }
      }
    }
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"disconnect sme failed:%s",e.what());
  }
  messages::SmeDisconnectResp resp;
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::LoggerGetCategories& msg)
{
  std::auto_ptr<const smsc::logger::Logger::LogLevels> cats(smsc::logger::Logger::getLogLevels());
  char * k;
  smsc::logger::Logger::LogLevel level;
  std::vector<messages::CategoryInfo> result;
  for (smsc::logger::Logger::LogLevels::Iterator i = cats->getIterator(); i.Next(k, level); )
  {
    messages::CategoryInfo ci;
    ci.setName(k);
    ci.setLevel(smsc::logger::Logger::getLogLevel(level));
    result.push_back(ci);
  }
  messages::LoggerGetCategoriesResp resp;
  resp.setCategories(result);
  prepareResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::LoggerSetCategories& msg)
{
  const std::vector<messages::CategoryInfo>& cats=msg.getCategories();
  smsc::logger::Logger::LogLevels levels;
  for(std::vector<messages::CategoryInfo>::const_iterator it=cats.begin(),end=cats.end();it!=end;it++)
  {
    levels[it->getName().c_str()] = smsc::logger::Logger::getLogLevel(it->getLevel().c_str());
  }
  smsc::logger::Logger::setLogLevels(levels);
  messages::LoggerSetCategoriesResp resp;
  prepareResp(msg,resp,0);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
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

void ControllerHandler::handle(const messages::AclGet& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=Smsc::getInstance().getAclMgr();
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
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::AclList& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=Smsc::getInstance().getAclMgr();
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
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::AclRemove& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=Smsc::getInstance().getAclMgr();
  messages::AclRemoveResp resp;
  int status=0;
  try{
    aclMgr->remove(msg.getAclId());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"failed to remove acl:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::AclCreate& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=Smsc::getInstance().getAclMgr();
  messages::AclCreateResp resp;
  int status=0;
  try{
    resp.setId(aclMgr->create2(msg.getName().c_str(),msg.getDescription().c_str(),msg.getAddresses(),(smsc::acls::AclCacheType)msg.getCacheType()));
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"failed to create acl:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::AclUpdate& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=Smsc::getInstance().getAclMgr();
  messages::AclUpdateResp resp;
  int status=0;
  try{
    const messages::AclInfo& ai=msg.getAcl();
    aclMgr->updateAclInfo(ai.getId(),ai.getName().c_str(),ai.getDescription().c_str(),(smsc::acls::AclCacheType)ai.getCacheType());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"failed to update acl:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::AclLookup& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=Smsc::getInstance().getAclMgr();
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
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::AclRemoveAddresses& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=Smsc::getInstance().getAclMgr();
  messages::AclRemoveAddressesResp resp;
  int status=0;
  try{
    const std::vector<std::string>& addrs=msg.getAddrs();
    for(std::vector<std::string>::const_iterator it=addrs.begin(),end=addrs.end();it!=end;it++)
    {
      aclMgr->removePhone(msg.getAclId(),*it);
    }
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"failed to remove addr from acl:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::AclAddAddresses& msg)
{
  smsc::acls::AclAbstractMgr* aclMgr=Smsc::getInstance().getAclMgr();
  messages::AclAddAddressesResp resp;
  int status=0;
  try{
    const std::vector<std::string>& addrs=msg.getAddrs();
    for(std::vector<std::string>::const_iterator it=addrs.begin(),end=addrs.end();it!=end;it++)
    {
      aclMgr->addPhone(msg.getAclId(),*it);
    }
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"failed to add addr to acl:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
/*
void ControllerHandler::handle(const messages::DlPrcList& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlPrcListResp resp;
  try{
    smsc::core::buffers::Array<smsc::distrlist::Principal> prcs=dladmin->getPrincipals();
    char buf[32];
    std::vector<messages::PrcInfo> info;
    for(int i=0;i<prcs.Count();i++)
    {
      smsc::distrlist::Principal& p=prcs[i];
      size_t bl=p.address.toString(buf,sizeof(buf));
      if(msg.hasAddress() && strncmp(msg.getAddress().c_str(),buf,std::min(bl,msg.getAddress().length()))!=0)
      {
        continue;
      }
      messages::PrcInfo pi;
      pi.setAddress(p.address.toString());
      pi.setMaxEl(p.maxEl);
      pi.setMaxLst(p.maxLst);
      info.push_back(pi);
    }
    resp.setResult(info);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.listPrc failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlPrcAdd& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlPrcAddResp resp;
  try{
    dladmin->addPrincipal(
        smsc::distrlist::Principal(
            msg.getPrc().getAddress().c_str(),
            msg.getPrc().getMaxLst(),
            msg.getPrc().getMaxEl()
            )
    );
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.addPrc failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlPrcDelete& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlPrcDeleteResp resp;
  try{
    dladmin->deletePrincipal(msg.getAddress().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.delPrc failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlPrcGet& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlPrcGetResp resp;
  try{
    const smsc::distrlist::Principal& p=dladmin->getPrincipal(msg.getAddress().c_str());
    messages::PrcInfo prc;
    prc.setAddress(p.address.toString());
    prc.setMaxEl(p.maxEl);
    prc.setMaxLst(p.maxLst);
    resp.setResult(prc);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.getPrc failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlPrcAlter& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlPrcAlterResp resp;
  try{
    dladmin->changePrincipal(
        smsc::distrlist::Principal(
            msg.getAddress().c_str(),
            msg.getMaxLists(),
            msg.getMaxElements()
            )
        );
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.alterPrc failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlMemAdd& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlMemAddResp resp;
  try{
    dladmin->addMember(msg.getDlName(),msg.getAddress().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.addMem failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlMemDelete& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlMemDeleteResp resp;
  try{
    dladmin->deleteMember(msg.getDlName(),msg.getAddress().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.delMem failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlMemGet& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlMemGetResp resp;
  try{
    buf::Array<smsc::sms::Address> members=dladmin->members(msg.getDlName());
    messages::string_list result;
    for(int i=0;i<members.Count();i++)
    {
      result.push_back(members[i].toString());
    }
    resp.setResult(result);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.getMem failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlSbmAdd& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlSbmAddResp resp;
  try{
    dladmin->grantPosting(msg.getDlName(),msg.getAddress().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.addSbm failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlSbmDel& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlSbmDelResp resp;
  try{
    dladmin->revokePosting(msg.getDlName(),msg.getAddress().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.sbmDel failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlSbmList& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlSbmListResp resp;
  try{
    buf::Array<smsc::sms::Address> sbm;
    dladmin->getSubmitters(msg.getDlName(),sbm);
    messages::string_list result;
    for(int i=0;i<sbm.Count();i++)
    {
      result.push_back(sbm[i].toString());
    }
    resp.setResult(result);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.listSbm failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlAdd& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlAddResp resp;
  try{
    dladmin->addDistrList(msg.getDlName(),!msg.hasOwner(),msg.hasOwner()?msg.getOwner().c_str():"0",msg.getMaxElements());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.addList failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlDelete& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlAddResp resp;
  try{
    dladmin->deleteDistrList(msg.getDlName());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.addList failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlGet& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlGetResp resp;
  try{
    smsc::distrlist::DistrList lst=dladmin->getDistrList(msg.getDlName());
    resp.setName(lst.name);
    if(!lst.system)
    {
      resp.setOwner(lst.owner.toString());
    }
    resp.setMaxEl(lst.maxEl);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.addList failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlList& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlListResp resp;
  try{
    std::vector<messages::DlListInfo> result;
    buf::Array<smsc::distrlist::DistrList> lst=dladmin->list();
    buf::Hash<int> nmflt;
    buf::Hash<int> ownflt;
    bool haveNmFlt=false;
    bool haveOwnFlt=false;
    if(!msg.getNames().empty())
    {
      for(messages::string_list::const_iterator it=msg.getNames().begin(),end=msg.getNames().end();it!=end;it++)
      {
        nmflt.Insert(it->c_str(),1);
        haveNmFlt=true;
      }
    }
    if(!msg.getOwners().empty())
    {
      for(messages::string_list::const_iterator it=msg.getOwners().begin(),end=msg.getOwners().end();it!=end;it++)
      {
        ownflt.Insert(it->c_str(),1);
        haveOwnFlt=true;
      }
    }
    char buf[32];
    for(int i=0;i<lst.Count();i++)
    {
      smsc::distrlist::DistrList& dl=lst[i];
      if(haveNmFlt && !nmflt.Exists(dl.name))continue;
      dl.owner.getText(buf,sizeof(buf));
      if(haveOwnFlt && (dl.system || !ownflt.Exists(buf)))continue;
      messages::DlListInfo dli;
      dli.setName(dl.name);
      if(!dl.system)
      {
        dli.setOwner(buf);
      }
      dli.setMaxEl(dl.maxEl);
      result.push_back(dli);
    }
    resp.setResult(result);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.listLists failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlAlter& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlAlterResp resp;
  try{
    dladmin->changeDistrList(msg.getDlName(),msg.getMaxElements());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.alterLst failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlCopy& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlCopyResp resp;
  try{
    dladmin->copyDistrList(msg.getSrcDlName(),msg.getDstDlName());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.copyLst failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DlRename& msg)
{
  smsc::distrlist::DistrListAdmin* dladmin=Smsc::getInstance().getDlAdmin();
  int status=0;
  messages::DlRenameResp resp;
  try{
    dladmin->renameDistrList(msg.getSrcDlName(),msg.getDstDlName());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"dl.renameLst failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}*/
void ControllerHandler::handle(const messages::CgmAddGroup& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  int status=0;
  messages::CgmAddGroupResp resp;
  try{
    cgm->AddGroup(msg.getId(),msg.getName().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"cgm.AddGroup failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::CgmDeleteGroup& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  int status=0;
  messages::CgmDeleteGroupResp resp;
  try{
    cgm->DeleteGroup(msg.getId());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"cgm.DeleteGroup failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::CgmAddAddr& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  int status=0;
  messages::CgmAddAddrResp resp;
  try{
    cgm->AddAddrToGroup(msg.getId(),msg.getAddr().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"cgm.AddAddr failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::CgmCheck& msg)
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
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::CgmDelAddr& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  int status=0;
  messages::CgmDelAddrResp resp;
  try{
    cgm->RemoveAddrFromGroup(msg.getId(),msg.getAddr().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"cgm.DelAddr failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::CgmAddAbonent& msg)
{
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();
  int status=0;
  messages::CgmAddAbonentResp resp;
  try{
    cgm->AddAbonent(msg.getId(),msg.getAddr().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"cgm.AddAbonent failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::CgmDelAbonent& msg)
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
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::CgmListAbonents& msg)
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
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::AliasAdd& msg)
{
  smsc::alias::AliasManager* aliaser=Smsc::getInstance().getAliaserInstance();
  int status=0;
  messages::AliasAddResp resp;
  try{
    smsc::alias::AliasInfo ai;
    ai.addr=msg.getAddr().c_str();
    ai.alias=msg.getAlias().c_str();
    ai.hide=msg.getHide();
    aliaser->addAlias(ai);
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"alias.add failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::AliasDel& msg)
{
  smsc::alias::AliasManager* aliaser=Smsc::getInstance().getAliaserInstance();
  int status=0;
  messages::AliasDelResp resp;
  try{
    aliaser->deleteAlias(msg.getAlias().c_str());
  }catch(std::exception& e)
  {
    status=1;
    smsc_log_warn(log,"alias.del failed:%s",e.what());
  }
  prepareResp(msg,resp,status);
  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::GetServicesStatus& msg)
{
  //!!!TODO!!!
  //  prepareResp(msg,resp,status);
  //  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::DisconnectService& msg)
{
  //!!!TODO!!!
  //  prepareResp(msg,resp,status);
  //  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
/*
void ControllerHandler::handle(const messages::MultipartMessageRequestResp& msg)
{
  //!!!TODO!!!
  //  prepareResp(msg,resp,status);
  //  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
void ControllerHandler::handle(const messages::ReplaceIfPresentRequestResp& msg)
{
  //!!!TODO!!!
  //  prepareResp(msg,resp,status);
  //  NetworkDispatcher::getInstance().enqueueMessage(resp);
}
*/
void ControllerHandler::handle(const messages::LockConfigResp& msg)
{
  NetworkDispatcher::getInstance().notifyOnMessageResp(msg.getSeqNum(),msg.getResp().getStatus());
  //!!!TODO!!!
  //  prepareResp(msg,resp,status);
  //  NetworkDispatcher::getInstance().enqueueMessage(resp);
}

void ControllerHandler::handle(const messages::UpdateProfileAbntResp& msg)
{
  smsc::smeman::SmscCommand cmd=smsc::smeman::SmscCommand::makeCommand(smsc::smeman::PROFILEUPDATERESP,msg.getSeqNum(),msg.getResp().getStatus(),0);
  Smsc::getInstance().getProfiler()->putCommand(cmd);
}

/*
void ControllerHandler::handle(const messages::DlMemAddAbntResp& msg)
{

}
void ControllerHandler::handle(const messages::DlMemDeleteAbntResp& msg)
{

}
void ControllerHandler::handle(const messages::DlSbmAddAbntResp& msg)
{

}
void ControllerHandler::handle(const messages::DlSbmDelAbntResp& msg)
{

}
void ControllerHandler::handle(const messages::DlAddAbntResp& msg)
{

}
void ControllerHandler::handle(const messages::DlDeleteAbntResp& msg)
{
}

void ControllerHandler::handle(const messages::DlCopyAbntResp& msg)
{

}
void ControllerHandler::handle(const messages::DlRenameAbntResp& msg)
{

}
*/


}
}
}
}
