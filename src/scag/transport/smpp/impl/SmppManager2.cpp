#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <map>
#include "SmppManager2.h"
#include "SmppStateMachine2.h"
#include "scag/transport/smpp/router/load_routes.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/exc/SCAGExceptions.h"
#include "util/xml/DOMTreeReader.h"
#include "util/xml/utilFunctions.h"
#include "scag/lcm/base/LongCallManager2.h"
#include "scag/sessions/base/SessionManager2.h"
#include "core/buffers/FixedLengthString.hpp"
#include "scag/counter/Manager.h"
#include "scag/counter/Accumulator.h"
#include "scag/counter/TimeSnapshot.h"
#include "sms/sms_const.h"

namespace {
const char* smppCounterName = "sys.traffic.global.smpp";
}

namespace scag2 {
namespace transport {
namespace smpp {

using namespace xercesc;
using namespace smsc::util;
using namespace smsc::util::xml;
using namespace exceptions;
using namespace config;
using namespace sessions;

time_t SmppManagerImpl::MetaEntity::expirationTimeout=300;
uint32_t SmppManagerImpl::smeUniqueId = 0;
const char* SmppManagerImpl::emptySystemId = "";


void SmppManagerImpl::shutdown()
{
    StopProcessing();
}

const int tag_smscParams=1024;

enum ParamTag{
tag_systemId,
tag_password,
tag_timeout,
tag_mode,
tag_enabled,
tag_providerId,
tag_snmpTracking,
tag_maxSmsPerSec,
tag_metaGroup,
tag_persistance,
tag_inQueueLimit,
tag_outQueueLimit,
tag_defaultLatin1,
tag_host=tag_outQueueLimit|tag_smscParams,
tag_port,
tag_althost,
tag_altport,
tag_uid,
tag_bindSystemId,
tag_bindPassword,
tag_addressRange,
tag_systemType
};

struct Param{
  const char* name;
  ParamTag tag;
};

#define TAGDEF(n) {#n,tag_##n}

static Param params[]=
{
TAGDEF(systemId),
TAGDEF(password),
TAGDEF(timeout),
TAGDEF(mode),
TAGDEF(providerId),
TAGDEF(maxSmsPerSec),
TAGDEF(host),
TAGDEF(port),
TAGDEF(althost),
TAGDEF(altport),
TAGDEF(enabled),
TAGDEF(snmpTracking),
TAGDEF(uid),
TAGDEF(bindSystemId),
TAGDEF(bindPassword),
TAGDEF(addressRange),
TAGDEF(systemType),
TAGDEF(inQueueLimit),
TAGDEF(outQueueLimit),
TAGDEF(metaGroup),
TAGDEF(persistance),
TAGDEF(defaultLatin1)
};

struct ParamsHash:public smsc::core::buffers::Hash<ParamTag>{
  ParamsHash()
  {
    for(unsigned i=0;i<sizeof(params)/sizeof(params[0]);i++)
    {
      Insert(params[i].name,params[i].tag);
    }
  }
};

static const ParamsHash paramsHash;

XmlStr GetNodeName( DOMNamedNodeMap* attr )
{
    return attr->getNamedItem(XmlStr("name"))->getNodeValue();
}

template <size_t SZ>
void FillStringValue(DOMNamedNodeMap* attr,buf::FixedLengthString<SZ>& str)
{
  XmlStr value(attr->getNamedItem(XmlStr("value"))->getNodeValue());
  str=value.c_str();
}

template <size_t SZ>
void FillStringValue(DOMNamedNodeMap* attr,char (&str)[SZ])
{
  XmlStr value(attr->getNamedItem(XmlStr("value"))->getNodeValue());
  strncpy(str,value.c_str(),SZ);
  str[SZ-1]=0;
}

int GetIntValue(DOMNamedNodeMap* attr)
{
  XmlStr value(attr->getNamedItem(XmlStr("value"))->getNodeValue());
  return atoi(value.c_str());
}

SmppBindType GetBindType(DOMNamedNodeMap* attr)
{
  buf::FixedLengthString<16> bindTypeName;
  FillStringValue(attr,bindTypeName);
  size_t len=strlen(bindTypeName.c_str());
  for(size_t i=0;i<len;i++)bindTypeName[i]=tolower(bindTypeName[i]);
  if     (!strcmp(bindTypeName.c_str(),"rx")) return btReceiver;
  else if(!strcmp(bindTypeName.c_str(),"tx")) return btTransmitter;
  else if(!strcmp(bindTypeName.c_str(),"trx"))return btTransceiver;
  throw smsc::util::Exception("Unknown bind mode:%s",bindTypeName.c_str());
}

bool GetBoolValue(DOMNamedNodeMap* attr)
{
  char bindTypeName[16];
  FillStringValue(attr,bindTypeName);
  size_t len=strlen(bindTypeName);
  for(size_t i=0;i<len;i++)bindTypeName[i]=tolower(bindTypeName[i]);
  if     (!strcmp(bindTypeName,"true")) return true;
  else if(!strcmp(bindTypeName,"false")) return false;
  XmlStr paramName(GetNodeName(attr));
  throw smsc::util::Exception("Invalid bool value for param '%s':%s",paramName.c_str(),bindTypeName);
}

static void FillEntity(SmppEntityInfo& entity,DOMNode* record)
{
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("smpp.man");
  for(DOMNode *node = record->getFirstChild(); node != 0; node = node->getNextSibling())
  {
    //DOMNode *node = children->item(i);
    if (node->getNodeType() != DOMNode::ELEMENT_NODE)continue;

    DOMNamedNodeMap *attrs = node->getAttributes();
    if(!attrs)
    {
      throw smsc::util::Exception("Invalid smpp.xml configuration file");
    }
    XmlStr paramName(GetNodeName(attrs));
    const ParamTag* tagPtr=paramsHash.GetPtr(paramName.c_str());
    if(!tagPtr)
    {
      throw smsc::util::Exception("Unknown parameter '%s' in smpp.xml",paramName.c_str());
    }
    if(entity.type==etService && *tagPtr>tag_smscParams)
    {
      throw smsc::util::Exception("Parameter '%s' allowed for smscrecord only",paramName.c_str());
    }
    switch(*tagPtr)
    {
      case tag_systemId:
        FillStringValue(attrs,entity.systemId);
        break;
      case tag_password:
        FillStringValue(attrs,entity.password);
        break;
      case tag_bindSystemId:
        FillStringValue(attrs,entity.bindSystemId);
        break;
      case tag_bindPassword:
        FillStringValue(attrs,entity.bindPassword);
        break;
      case tag_timeout:
        entity.timeOut=GetIntValue(attrs);
        break;
      case tag_mode:
        entity.bindType=GetBindType(attrs);
        break;
      case tag_host:
        FillStringValue(attrs,entity.host);
        break;
      case tag_port:
        entity.port=GetIntValue(attrs);
        break;
      case tag_althost:
        FillStringValue(attrs,entity.altHost);
        break;
      case tag_altport:
        entity.altPort=GetIntValue(attrs);
        break;
      case tag_enabled:
        entity.enabled=GetBoolValue(attrs);
        break;
      case tag_snmpTracking:
        entity.snmpTracking = GetBoolValue(attrs);
        break;
      case tag_defaultLatin1:
        entity.defaultLatin1=GetBoolValue(attrs);
        break;
      case tag_uid:
        entity.uid=GetIntValue(attrs);
        break;
      case tag_addressRange:
        FillStringValue(attrs,entity.addressRange);
        break;
      case tag_systemType:
        FillStringValue(attrs,entity.systemType);
        break;
      case tag_maxSmsPerSec:
        entity.sendLimit=GetIntValue(attrs);
        break;
      case tag_inQueueLimit:
        entity.inQueueLimit=GetIntValue(attrs);
        break;
      case tag_outQueueLimit:
        entity.outQueueLimit=GetIntValue(attrs);
        break;
      case tag_metaGroup:
        FillStringValue(attrs,entity.metaGroupId);
        break;
      default:
        smsc_log_warn(log,"Parameter '%s' meaningless for record type %s. Skipped",paramName.c_str(),entity.type==etService?"service":"smsc");
    }
  }
}

static void ParseTag(SmppManagerImpl* smppMan,DOMNodeList* list,SmppEntityType et)
{
  using namespace smsc::util::xml;
  size_t listLength = list->getLength();
  for (size_t i=0; i<listLength; i++)
  {
    SmppEntityInfo entity;
    entity.type=et;
    //bool enabled=false;
    DOMNode *record = list->item(i);
    //DOMNodeList *children = record->getChildNodes();
    //unsigned childrenCount= children->getLength();
    //for (unsigned j=0; j<childrenCount; j++)
    FillEntity(entity,record);
    smppMan->addSmppEntity(entity);
    if(entity.metaGroupId.length())
    {
      smppMan->addMetaEndPoint(entity.metaGroupId.c_str(),entity.systemId.c_str());
    }
  }
}

BalancingPolicy GetPolicyValue(DOMNamedNodeMap* attrs)
{
  char policy[64];
  FillStringValue(attrs,policy);
  if(strcmp(policy,"roundrobin")==0)return bpRoundRobin;
  if(strcmp(policy,"random")==0)return bpRandom;
  throw Exception("Invalid balancing policy value:%s",policy);
}

static void FillMetaEntity(MetaEntityInfo& entity,DOMNode* record)
{
  for(DOMNode *node = record->getFirstChild(); node != 0; node = node->getNextSibling())
  {
    if (node->getNodeType() != DOMNode::ELEMENT_NODE)continue;

    DOMNamedNodeMap *attrs = node->getAttributes();
    if(!attrs)
    {
      throw smsc::util::Exception("Invalid smpp.xml configuration file");
    }
    XmlStr paramName(GetNodeName(attrs));
    if(paramName=="policy")
    {
      entity.policy=GetPolicyValue(attrs);
      continue;
    }
    if(paramName=="systemId")
    {
      FillStringValue(attrs,entity.systemId);
      continue;
    }
    if(paramName=="persistence")
    {
      entity.persistanceEnabled=GetBoolValue(attrs);
      continue;
    }
    throw Exception("Unknown meta entity parameter name:'%s'",paramName.c_str());
  }
}

static void ParseMetaTag(SmppManagerImpl* smppMan,DOMNodeList* list,MetaEntityType et)
{
  using namespace smsc::util::xml;
  size_t listLength = list->getLength();
  for (size_t i=0; i<listLength; i++)
  {
    MetaEntityInfo entity;
    entity.type=et;
    DOMNode *record = list->item(i);
    FillMetaEntity(entity,record);
    smppMan->addMetaEntity(entity);
  }
}


SmppManagerImpl::SmppManagerImpl( snmp::TrapRecordQueue* snmpqueue ) :
ConfigListener(SMPPMAN_CFG), sm(this,this),
// licenseCounter(counter::Manager::getInstance().createCounter(::smppCounterName,
testRouter_(0),
snmpqueue_(snmpqueue),
queueLimit(1000)
{
  log=smsc::logger::Logger::getInstance("smpp.man");
  limitsLog=smsc::logger::Logger::getInstance("smpp.lmt");
  log_dump=smsc::logger::Logger::getInstance("smpp.man.dmp");
    {
        counter::Manager& mgr = counter::Manager::getInstance();
        // const unsigned maxsms = ConfigManager::Instance().getLicense().maxsms;
        counter::ObserverPtr o = mgr.getObserver(::smppCounterName);
        licenseCounter =
            mgr.registerAnyCounter( new counter::TimeSnapshot(::smppCounterName,10,20,o.get()) );
    }
  running=false;
  lastUid=0;
  lastExpireProcess=0;
  lcmProcessingCount = 0;
    queuedCmdCount = 0;
  lastLicenseExpTest=0;
  licenseFileCheckHour=0;
}

SmppManagerImpl::~SmppManagerImpl()
{
    StopProcessing();
    {
        MutexGuard mg(queueMon);
        if ( queue.Count() != 0 || respQueue.Count() != 0 || lcmQueue.Count() != 0 ) {
            smsc_log_error( log, "queues have commands: %u/%u/%u",
                            queue.Count(), respQueue.Count(), lcmQueue.Count() );
        }
    }
    {
        // cleaning up metaregistry
        char* id;
        MetaEntity* pme;
        for ( smsc::core::buffers::Hash< MetaEntity* >::Iterator it(&metaRegistry);
              it.Next(id,pme); ) {
            delete pme;
        }
        metaRegistry.Empty();
    }
}

void SmppManagerImpl::Init(const char* cfgFile)
{
  if ( !licenseCounter.get() )
    throw SCAGException("license counter %s cannot be created",::smppCounterName);

  smeUniqueId = 0;

  cfgFileName=cfgFile;
  using namespace smsc::util::xml;
  DOMTreeReader reader;
  DOMDocument* doc=reader.read(cfgFile);
  DOMElement *elem = doc->getDocumentElement();

    // initialization of sm
    unsigned socketsPerThread = getUnsigned( "smpp.core.socketsPerMultiplexer", 16 );
    unsigned bindTimeout = getUnsigned( "smpp.core.bindWaitTimeout", 10 );
    unsigned connectionsPerIp = getUnsigned( "smpp.core.connectionsPerIp", 100 );
    unsigned failTimeout = getUnsigned( "smpp.core.ipBlockingTime", 60 );
    unsigned maxMultiCount = getUnsigned( "smpp.core.maxRWMultiplexersCount", 100 );
    sm.init( socketsPerThread, bindTimeout, connectionsPerIp, failTimeout, maxMultiCount );

    // collecting whitelisted ips
    try {
        ConfigView cv(*ConfigManager::Instance().getConfig(),"smpp.core.whitelist");
        typedef std::set< std::string > CStrSet;
        std::auto_ptr<CStrSet> hostset(cv.getStrParamNames());
        bool hostok = false;
        if ( hostset.get() ) {
            for ( CStrSet::const_iterator i = hostset->begin();
                  i != hostset->end();
                  ++i ) {
                std::string hostip;
                try {
                    std::auto_ptr<char> hostwip(cv.getString(i->c_str()));
                    sm.addWhiteIp(hostwip.get());
                    hostok = true;
                } catch ( ConfigException&) {
                    smsc_log_warn(log,"cannot add whitelisted host: %s", i->c_str());
                }
                // sm.addWhiteIp( i->c_str() );
            }
        }
        if ( ! hostok ) {
            sm.addWhiteIp("127.0.0.1");
        }
    } catch (std::exception& e) {
        smsc_log_info(log,"whitelist exc: %s", e.what());
    } catch (...) {
        smsc_log_info(log,"whitelist exc: unknown");
    }

  LoadRoutes("conf/smpp_routes.xml");

  DOMNodeList *list = elem->getElementsByTagName(XmlStr("metasmerecord"));
  ParseMetaTag(this,list,mtMetaService);

  list = elem->getElementsByTagName(XmlStr("metasmscrecord"));
  ParseMetaTag(this,list,mtMetaSmsc);

  list = elem->getElementsByTagName(XmlStr("smerecord"));
  ParseTag(this,list,etService);

  list = elem->getElementsByTagName(XmlStr("smscrecord"));
  ParseTag(this,list,etSmsc);

  running=true;

  int stmCnt = 0;
  try
  {
      stmCnt = ConfigManager::Instance().getConfig()->getInt("smpp.core.state_machines_count");
  } catch (HashInvalidKeyException& e)
  {
      running=false;
      throw SCAGException("Invalid parameter 'smpp.core.state_machines_count'");
  }

  queueLimit=1000;
  try{
    queueLimit= ConfigManager::Instance().getConfig()->getInt("smpp.core.eventQueueLimit");
  }catch(HashInvalidKeyException& e)
  {
    smsc_log_warn(log,"smpp.queueLimit not found! Using default(%d)",queueLimit);
  }
    {
        const char* cname = "sys.smpp.queue.global";
        counter::Manager& mgr = counter::Manager::getInstance();
        counter::ObserverPtr o = mgr.getObserver(cname);
        queueCount = mgr.registerAnyCounter(new counter::Accumulator(cname,o.get()));
        if (!queueCount.get()) {
            throw SCAGException("cannot create counter '%s'",cname);
        }
        // queueCount->setMaxVal(queueLimit);
    }

  /*
  try{
    const char* tags=ConfigManager::Instance().getConfig()->getString("smpp.transitOptionalTags");
    size_t n=0;
    int i=0;
    size_t len=strlen(tags);
    int tag;
    while(n<len)
    {
      if(sscanf(tags+n,"%x%n",&tag,&i)==1)
      {
        StateMachine::addTransitOptional(tag);
      }
    else
    {
      smsc_log_warn(log, "Failed to parse tags list :'%s'",tags+n);
    break;
    }
    n+=i;
      if(tags[n]==',')
      {
        n++;
      }
      else if(tags[n])
      {
        smsc_log_warn(log, "unexpected symbol in list of transit optional tags:'%c'", tags[n]);
        break;
      }
    }
  }catch(...)
  {
    smsc_log_warn(log, "smpp.transitOptionalTags not found in config");
  }
   */

    {
        const unsigned defaultRespTimeout = 70;
        unsigned respTimeout;
        try{
            respTimeout = unsigned(ConfigManager::Instance().getConfig()->getInt("smpp.core.respTimeout"));
        } catch(HashInvalidKeyException& e) {
            respTimeout = defaultRespTimeout;
            smsc_log_warn(log,"smpp.core.respTimeout is not found! Using default (%u)",
                          respTimeout);
        }
        if ( respTimeout < 60 ) {
            smsc_log_warn(log,"smpp.core.respTimeout is too small (%u), using %u",
                          respTimeout,defaultRespTimeout);
            respTimeout = defaultRespTimeout;
        } else if ( respTimeout > 1000 ) {
            smsc_log_warn(log,"smpp.core.respTimeout is too big (%u), using %u",
                          respTimeout,defaultRespTimeout);
            respTimeout = defaultRespTimeout;
        }
        StateMachine::initRegistry(respTimeout);
    }
    
  smsc_log_info(log,"Starting %d state machines",stmCnt);
  for(int i=0;i<stmCnt;i++)
  {
    tp.startTask(new StateMachine(this,this));
  }

    sm.start();
}

bool SmppManagerImpl::LoadEntityFromConfig(SmppEntityInfo& info,const char* sysId,SmppEntityType et)
{
  using namespace smsc::util::xml;
  DOMTreeReader reader;
  DOMDocument* doc=reader.read(cfgFileName.c_str());
  DOMElement *elem = doc->getDocumentElement();

  DOMNodeList *list;
  if(et==etService)
  {
    list = elem->getElementsByTagName(XmlStr("smerecord"));
  }else
  {
    list = elem->getElementsByTagName(XmlStr("smscrecord"));
  }
  size_t listLength = list->getLength();
  for (size_t i=0; i<listLength; i++)
  {
    SmppEntityInfo entity;
    entity.type=et;
    DOMNode *record = list->item(i);
    FillEntity(entity,record);
    if(entity.systemId==sysId)
    {
      info=entity;
      return true;
    }
  }
  return false;
}

bool SmppManagerImpl::LoadMetaEntityFromConfig(MetaEntityInfo& info,const char* sysId)
{
  using namespace smsc::util::xml;
  DOMTreeReader reader;
  DOMDocument* doc=reader.read(cfgFileName.c_str());
  DOMElement *elem = doc->getDocumentElement();

  DOMNodeList *list= elem->getElementsByTagName(XmlStr("metasmerecord"));
  size_t listLength = list->getLength();
  for (size_t i=0; i<listLength; i++)
  {
    MetaEntityInfo entity;
    entity.type=mtMetaService;
    DOMNode *record = list->item(i);
    FillMetaEntity(entity,record);
    if(entity.systemId==sysId)
    {
      info=entity;
      return true;
    }
  }
  list = elem->getElementsByTagName(XmlStr("metasmscrecord"));
  listLength = list->getLength();
  for (unsigned i=0; i<listLength; i++)
  {
    MetaEntityInfo entity;
    entity.type=mtMetaSmsc;
    DOMNode *record = list->item(i);
    FillMetaEntity(entity,record);
    if(entity.systemId==sysId)
    {
      info=entity;
      return true;
    }
  }
  return false;
}


void SmppManagerImpl::configChanged()
{
}

void SmppManagerImpl::LoadRoutes(const char* cfgFile)
{
  RouteConfig& cfg = ConfigManager::Instance().getRouteConfig();
  router::RouteManager* newman=new router::RouteManager();
  router::loadRoutes(newman,cfg,0);
    if (log_dump->isDebugEnabled()) {
        std::vector<std::string> dump;
        newman->dumpInto( dump );
        std::string dumpstr;
        dumpstr.reserve(220*dump.size()); //yes strings are long
        for ( std::vector<std::string>::const_iterator i = dump.begin();
              i != dump.end();
              ++i ) {
            dumpstr += "\n" + *i;
        }
        smsc_log_debug(log_dump,"new routes: %s",dumpstr.c_str());
    }
  {
    sync::MutexGuard mg(routerSwitchMtx);
    routeMan=newman;
  }
}

void SmppManagerImpl::ReloadRoutes()
{
    RouteConfig& cfg = ConfigManager::Instance().getRouteConfig();
  router::RouteManager* newman=new router::RouteManager();
  router::loadRoutes(newman,cfg,0);
    if (log_dump->isDebugEnabled()) {
        std::vector<std::string> dump;
        newman->dumpInto( dump );
        std::string dumpstr;
        dumpstr.reserve(220*dump.size());
        for ( std::vector<std::string>::const_iterator i = dump.begin();
              i != dump.end();
              ++i ) {
            dumpstr += "\n" + *i;
        }
        smsc_log_debug(log_dump,"new routes: %s",dumpstr.c_str());
    }
  {
    sync::MutexGuard mg(routerSwitchMtx);
    routeMan=newman;
  }
}

void SmppManagerImpl::addRegistryItem(SmppEntityInfo& info)
{
  info.uniqueId = ++smeUniqueId;
  const char* tmp = new char[smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1];
  strncpy((char*)tmp, info.systemId.c_str(), 34);
  registry.Insert(tmp, new SmppEntity(info));
  idx_registry.Insert(info.uniqueId, tmp);
}

uint32_t SmppManagerImpl::getSmeIndex(const char* smeSystemId)
{
  SmppEntity** ptr = registry.GetPtr(smeSystemId);
  return ptr ? (*ptr)->info.uniqueId : 0;
}

const char* SmppManagerImpl::getSmeSystemId(uint32_t smeIndex)
{
  const char** ptr = 0;
  const char* result = 0;
  try
  {
    ptr = idx_registry.GetPtr(smeIndex);
    result = ptr ? *ptr : result;
  }
  catch (...)
  {
    result = emptySystemId;
    smsc_log_debug(log_dump,"getSmeSystemId(%d) not found in idx_registry", smeIndex);
  }
  return result;
}

void SmppManagerImpl::addSmppEntity(const SmppEntityInfo& info)
{
  smsc_log_debug(log,"addSmppEntity:%s",info.systemId.c_str());
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr = registry.GetPtr(info.systemId.c_str());
  if (ptr)
  {
    if((**ptr).info.type != etUnknown)
    {
      throw smsc::util::Exception("Duplicate systemId='%s'", info.systemId.c_str());
    }
    (**ptr).info = info;
  }
  else
  {
    addRegistryItem((SmppEntityInfo&)info);
  }
  if(info.type==etSmsc && info.enabled)
  {
    SmscConnectInfo ci;
    ci.regSysId=info.systemId.c_str();
    ci.sysId=info.bindSystemId.c_str();
    ci.pass=info.bindPassword.c_str();
    ci.hosts[0]=info.host.c_str();
    ci.ports[0]=info.port;
    ci.hosts[1]=info.altHost.c_str();
    ci.ports[1]=info.altPort;
    ci.addressRange=info.addressRange.c_str();
    ci.systemType=info.systemType.c_str();
    sm.getSmscConnectorAdmin()->addSmscConnect(ci);
  }
}

void SmppManagerImpl::updateSmppEntity(const SmppEntityInfo& info)
{
  smsc_log_debug(log,"updateSmppEntity:%s", info.systemId.c_str());
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr = registry.GetPtr(info.systemId.c_str());
  if(!ptr)
  {
    throw smsc::util::Exception("updateSmppEntity:Enitity with systemId='%s' not found", info.systemId.c_str());
  }

  SmppEntity& ent = **ptr;
  const char** tmp = idx_registry.GetPtr(ent.info.uniqueId);
  if (tmp)
  {
    strncpy((char*)*tmp, ent.info.systemId.c_str(), smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
  }
  else
  {
    idx_registry.Insert(ent.info.uniqueId, ent.info.systemId.c_str());
    smsc_log_debug(log_dump, "updateSmppEntity error: smeIndex(%d) not found in idx_registry, sme=%s", ent.info.uniqueId, ent.info.systemId.c_str());
  }
  const bool oldEnabled = ent.updateInfo( info );

  // MutexGuard emg(ent.mtx);
  // bool oldEnabled=ent.info.enabled;
  // ent.info=info;

  if (info.type==etSmsc) {
    SmscConnectInfo ci;
    ci.regSysId = info.systemId.c_str();
    ci.sysId = info.bindSystemId.c_str();
    ci.pass = info.bindPassword.c_str();
    ci.hosts[0] = info.host.c_str();
    ci.ports[0] = info.port;
    ci.hosts[1] = info.altHost.c_str();
    ci.ports[1] = info.altPort;
    ci.addressRange = info.addressRange.c_str();
    ci.systemType = info.systemType.c_str();

    if (oldEnabled==false && info.enabled==true) {
      sm.getSmscConnectorAdmin()->addSmscConnect(ci);
    } else if(oldEnabled==true && info.enabled==false) {
      sm.getSmscConnectorAdmin()->deleteSmscConnect(info.systemId.c_str());
    } else if(oldEnabled==true && info.enabled==true) {
      sm.getSmscConnectorAdmin()->updateSmscConnect(ci);
    }
  }

    /*
  switch(ent.bt)
  {
    case btTransceiver:
      ent.channel->disconnect();
      break;
    case btTransmitter:
      ent.transChannel->disconnect();
      break;
    case btRecvAndTrans:
      ent.transChannel->disconnect();
      //fallthru
    case  btReceiver:
      ent.recvChannel->disconnect();
      break;
  default:
      // nothing
      break;
  }
     */
  ent.disconnect();
}

void SmppManagerImpl::disconnectSmppEntity(const char* sysId)
{
  smsc_log_debug(log,"disconnectSmppEntity:%s",sysId);
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr=registry.GetPtr(sysId);
  if(!ptr)
  {
    throw smsc::util::Exception("disconnectSmppEntity:Enitity with systemId='%s' not found",sysId);
  }
  SmppEntity& ent=**ptr;
  ent.disconnect();
  /*
  MutexGuard emg(ent.mtx);
  switch(ent.bt)
  {
    case btTransceiver:
      ent.channel->disconnect();
      break;
    case btTransmitter:
      ent.transChannel->disconnect();
      break;
    case btRecvAndTrans:
      ent.transChannel->disconnect();
      //fallthru
    case  btReceiver:
      ent.recvChannel->disconnect();
      break;
  default:
      // nothing
      break;
  }
   */

    if (ent.info.type==etSmsc)
        sm.getSmscConnectorAdmin()->reportSmscDisconnect(sysId);
}

void SmppManagerImpl::deleteSmppEntity(const char* sysId)
{
  smsc_log_debug(log,"deleteSmppEntity:%s",sysId);
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr=registry.GetPtr(sysId);
  if(!ptr)
  {
    throw smsc::util::Exception("deleteSmppEntity:Enitity with systemId='%s' not found", sysId);
  }
    SmppEntity& ent=**ptr;
    ent.disconnect();
    /*
  MutexGuard emg(ent.mtx);
  switch(ent.bt)
  {
    case btTransceiver:
      ent.channel->disconnect();
      break;
    case btTransmitter:
      ent.transChannel->disconnect();
      break;
    case btRecvAndTrans:
      ent.transChannel->disconnect();
      //fallthru
    case  btReceiver:
      ent.recvChannel->disconnect();
      break;
  default:
      // nothing
      break;
  }
     */

    if(ent.info.type==etSmsc)
    {
        sm.getSmscConnectorAdmin()->deleteSmscConnect(sysId);
    }
    ent.reset();
    // ent.info.type = etUnknown;
    /*
  ent.bt=btNone;
  ent.channel=0;
  ent.transChannel=0;
  ent.recvChannel=0;
  ent.seq=0;
  ent.info.type=etUnknown;
     */
}

SmppEntityAdminInfoList * SmppManagerImpl::getEntityAdminInfoList(SmppEntityType entType)
{
    MutexGuard mg(regMtx);

    SmppEntity * value = 0;
    char * key = 0;

    SmppEntityAdminInfoList * result = new SmppEntityAdminInfoList;

    SmppEntityAdminInfo ai;
    for (buf::Hash<SmppEntity *>::Iterator it = registry.getIterator(); it.Next(key, value);)
    {
        if ( value->getAdminInfo(entType,ai) ) {
            result->push_back(ai);
        }
        // MutexGuard emg(value->mtx);
        // if (value->info.type == entType)
        // {
        // SmppEntityAdminInfo ai = {value->getSystemId(), value->bt, value->info.host, value->info.port, value->connected};
        // result->push_back(ai);
        // }
    }

    return result;
}



int SmppManagerImpl::registerSmeChannel( const char* sysId,
                                         const char* pwd,
                                         SmppBindType bt,
                                         SmppChannel* ch )
{
    const char* text = 0;
    int ret = rarFailed;
    bool snmpTracking = true;
    do {
        if (!ch) {
            smsc_log_warn(log,"failed to register sme with sysId='%s' - ch is null",sysId);
            text = "chanell is null";
            break;
        }

        sync::MutexGuard mg(regMtx);
        SmppEntity** ptr=registry.GetPtr(sysId);
        if (!ptr)
        {
            smsc_log_info(log,"Failed to register sme %p with sysId='%s' - Not found",
                          ch, sysId);
            // TODO: We have no SME info information here,
            // so we cannot determine if we should report on this failure via SNMP.
            // Right now, we do report about this activity.
            text = "not found";
            break;
        }
        SmppEntity& ent=**ptr;
        snmpTracking = ent.info.snmpTracking;
        try {
            text = ent.checkAndBindChannel(etService,bt,ch,++lastUid,pwd);
            if (!text) {
                text = "connected";
                // smsc_log_info(log,"registerSmeChannel('%s') successfully connected",sysId);
                ret = rarOk;
            } else if ( !text[0] ) {
                // smsc_log_info(log,"registerSmeChannel('%s') failed: already connected",sysId);
                text = 0; // reset text to avoid snmp tracking
                ret = rarAlready;
            }
            const smsc::logger::Logger::LogLevel level = 
                ( ret == rarFailed ?
                  smsc::logger::Logger::LEVEL_WARN :
                  smsc::logger::Logger::LEVEL_INFO );
            if (log->isLogLevelEnabled(level)) {
                log->log(level,"registerSmeChannel(%p,'%s'): %s",
                         ch,sysId,text ? text : "already connected");
            }
        } catch (std::exception& e) {
            smsc_log_warn(log,"registerSmeChannel(%p,'%s') exc: %s",ch,sysId,e.what());
            text = "unknown error";
        }

        /*
        if(!(*ptr)->info.enabled)
        {
            smsc_log_info(log,"Failed to register sme with sysId='%s' - disabled",sysId);
            text = "sme is disabled";
            break;
        }
        if(!(ent.info.password == pwd))
        {
            smsc_log_info(log,"Failed to register sme with sysId='%s' - password mismatch:'%s'!='%s'",sysId,ent.info.password.c_str(),pwd);
            text = "sme password mismatch";
            break;
        }
        {
            MutexGuard mg2(ent.mtx);
            if(ent.info.type==etUnknown)
            {
                smsc_log_warn(log,"Failed to register sme with sysId='%s' - Entity deleted",sysId);
                text = "sme was deleted";
                break;
            }
            if(ent.info.type!=etService)
            {
                smsc_log_warn(log,"Failed to register sme with sysId='%s' - Entity type is not sme",sysId);
                text = "sme type is not sme";
                break;
            }
            
            if((ent.info.bindType==btReceiver && bt!=btReceiver)||
               (ent.info.bindType==btTransmitter && bt!=btTransmitter))
            {
                smsc_log_info(log,"Failed to register sme with sysId='%s' - Invalid bind type",sysId);
                text = "sme bind type is invalid";
                break;
            }
            if((ent.bt==btReceiver && bt==btReceiver)||
               (ent.bt==btTransmitter && bt==btTransmitter)||
               (ent.bt!=btNone))
            {
                smsc_log_info(log,"Failed to register sme with sysId='%s' - Already registered",sysId);
                ret = rarAlready;
                break;
            }

            if(ent.bt==btNone) {
                ent.bt=bt;
            } else {
                ent.bt=btRecvAndTrans;
            }

            if (bt==btTransceiver) {
                ent.channel=ch;
            } else if(bt==btReceiver) {
                ent.recvChannel=ch;
            } else if(bt==btTransmitter) {
                ent.transChannel=ch;
            }
        }
        ent.info.host = ch->getPeer();
        ent.connected = true;
        ent.setUid(++lastUid);
        smsc_log_info(log,"Registered sme with sysId='%s'",sysId);
        text = "connected";
        ret = rarOk;
         */
    } while ( false );

    if ( text && snmpqueue_ && snmpTracking ) {
        snmp::TrapRecord* trap = new snmp::TrapRecord;
        trap->recordType = snmp::TrapRecord::Trap;
        trap->status = ( ret == rarFailed ? snmp::TrapRecord::TRAPTNEWALERT : snmp::TrapRecord::TRAPTCLRALERT );
        trap->id = sysId;
        trap->category = "SME";
        trap->severity = ( ret == rarFailed ? snmp::TrapRecord::MAJOR : snmp::TrapRecord::CLEAR );
        trap->text = text;
        snmpqueue_->Push( trap );
    }
    return ret;
}

int SmppManagerImpl::registerSmscChannel(SmppChannel* ch)
{
    const char* text = 0;
    int ret = rarFailed;
    bool snmpTracking = true;
    do {
        assert(ch);

        sync::MutexGuard mg(regMtx);
        SmppEntity** ptr=registry.GetPtr(ch->getSystemId());
        if (!ptr) {
            // smsc_log_info(log,"Failed to register smsc with sysId='%s' - Not found",ch->getSystemId());
            text = "not found";
            break;
        }
        SmppEntity& ent=**ptr;
        snmpTracking = ent.info.snmpTracking;
        try {
            text = ent.checkAndBindChannel( etSmsc, btTransceiver, ch, ++lastUid, 0 );
            if (!text) {
                text = "connected";
                ret = rarOk;
            } else if (!text[0]) {
                text = 0;
                ret = rarAlready;
            }
            const smsc::logger::Logger::LogLevel level = 
                ( ret == rarFailed ?
                  smsc::logger::Logger::LEVEL_WARN :
                  smsc::logger::Logger::LEVEL_INFO);
            if (log->isLogLevelEnabled(level)) {
                log->log(level,"registerSmscChannel(%p,'%s'): %s",
                         ch,ch->getSystemId(),text ? text : "already connected");
            }
        } catch (std::exception& e) {
            smsc_log_warn(log,"registerSmscChannel(%p,'%s') exc: %s",
                          ch,ch->getSystemId(),e.what());
            text = "unknown error";
        }

        /*
        if(!(**ptr).info.enabled)
        {
            smsc_log_info(log,"Failed to register smsc with sysId='%s' - disabled",ch->getSystemId());
            text = "smsc is disabled";
            break;
        }
        SmppEntity& ent=**ptr;
        {
            MutexGuard mg2(ent.mtx);
            ent.bt=btTransceiver;
            ent.channel=ch;
        }
        smsc_log_info(log,"Registered smsc connection with sysId='%s'",ch->getSystemId());
        ent.connected = true;
        ent.setUid(++lastUid);
        text = "connected";
        ret = rarOk;
         */
    } while ( false );

    if ( text && snmpqueue_ && snmpTracking ) {
        std::auto_ptr<snmp::TrapRecord> trap(new snmp::TrapRecord);
        trap->recordType = snmp::TrapRecord::Trap;
        trap->status = ( ret == rarFailed ? snmp::TrapRecord::TRAPTNEWALERT : snmp::TrapRecord::TRAPTCLRALERT );
        trap->id = ch->getSystemId();
        trap->category = "SMSC";
        trap->severity = ( ret == rarFailed ? snmp::TrapRecord::MAJOR : snmp::TrapRecord::CLEAR );
        trap->text = text;
        snmpqueue_->Push( trap.release() );
    }
    return ret;
}

void SmppManagerImpl::unregisterChannel(SmppChannel* ch)
{
//    smsc_log_debug(log, "SmppManagerImpl::unregisterChannel %p %s", ch, ch->getSystemId());
    const char* text = 0;
    bool isSME = false;
    bool snmpTracking = true;
    bool isDeleted = false;  // if the sme was deleted (i.e. the unregister is legal)
    do {
        sync::MutexGuard mg(regMtx);
        SmppEntity** ptr=registry.GetPtr(ch->getSystemId());
        if (!ptr) {
            smsc_log_info(log,"Failed to unregister %p sysId='%s' - Not found",ch,ch->getSystemId());
            break; // no snmp
        }
        SmppEntity& ent=**ptr;
        snmpTracking = ent.info.snmpTracking;
        isDeleted = !ent.info.enabled;
        isSME = (ent.info.type == etService);
        if (ent.unbindChannel(ch)) {
            text = "disconnected";
        } else {
            smsc_log_warn(log,"attempt to unregister channel with invalid bind type %p sysId='%s'",ch,ch->getSystemId());
        }
    } while ( false );

    if ( text && snmpqueue_ && snmpTracking ) {
        snmp::TrapRecord* trap = new snmp::TrapRecord;
        trap->recordType = snmp::TrapRecord::Trap;
        trap->status = isDeleted ? snmp::TrapRecord::TRAPTCLRALERT : snmp::TrapRecord::TRAPTNEWALERT;
        trap->id = ch->getSystemId();
        trap->category = ( isSME ? "SME" : "SMSC" );
        trap->severity = isDeleted ? snmp::TrapRecord::CLEAR : snmp::TrapRecord::MAJOR;
        trap->text = text;
        snmpqueue_->Push( trap );
    }
}

void SmppManagerImpl::putCommand( SmppChannel& ct, std::auto_ptr<SmppCommand> cmd )
{
    SmppEntity* entPtr=0;
    {
        MutexGuard regmg(regMtx);
        SmppEntity** ptr=registry.GetPtr(ct.getSystemId());
        if (!ptr) throw Exception("Unknown system id:%s",ct.getSystemId());
        cmd->setEntity(*ptr);
        entPtr=*ptr;
    }

    int i = cmd->getCommandId();
    if ( i == DELIVERY_RESP || i == SUBMIT_RESP || i == DATASM_RESP ) {
        MutexGuard mg(queueMon);
        respQueue.Push( cmd.release() );
        queueMon.notify();
        return;
    }

    if(!running)
    {
        smsc_log_warn(limitsLog,"Denied %s from '%s' due to shutting down", i == DELIVERY ? "DELIVERY" : (i == SUBMIT ? "SUBMIT" : "DATASM"), entPtr->info.systemId.c_str());
        std::auto_ptr<SmppCommand> resp = SmppCommand::mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::SYSERR);
        ct.putCommand( resp );
        return;
    }

    MutexGuard mg(queueMon);
    int licLimit=ConfigManager::Instance().getLicense().maxsms;
    // NOTE: counter already gives value averaged to the number of seconds
    // this call actually does not increment, but adjust current time.
    const int cntValue = int(licenseCounter->increment(0,0));
    if(cntValue>licLimit)
    {
        bool allow=false;
        if(i==SUBMIT || i==DELIVERY || i==DATASM)
        {
            SMS& sms=*cmd->get_sms();
            if(sms.hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP))
            {
                if( sms.getIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP) != uint32_t(smsc::sms::USSD_PSSR_IND) )
                {
                    allow=true;
                }
            }
        }
        if(!allow)
        {
            smsc_log_info(limitsLog,"Denied by license limitation:%d/%d",cntValue,licLimit);
            std::auto_ptr<SmppCommand> resp = SmppCommand::mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::THROTTLED);
            ct.putCommand(resp);
            return;
        }
    }
    
    // int cnt = entPtr->incCnt.Get();
    unsigned cnt = unsigned(entPtr->incCnt->increment(0,0));
    if (entPtr->info.sendLimit>0 && 
        cnt >= unsigned(entPtr->info.sendLimit) &&
        cmd->getCommandId()==SUBMIT &&
        !cmd->get_sms()->hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP))
    {
        smsc_log_warn(limitsLog,"Denied submit from '%s' by sendLimit:%d/%d",entPtr->info.systemId.c_str(),cnt,entPtr->info.sendLimit);
        std::auto_ptr<SmppCommand> resp = SmppCommand::mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::MSGQFUL);
        ct.putCommand(resp);
    } else if ( queue.Count()>=queueLimit && !cmd->get_sms()->hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP)) {
        smsc_log_warn(limitsLog,"Denied submit from '%s' by queueLimit:%d/%d",entPtr->info.systemId.c_str(),queue.Count(),queueLimit);
        std::auto_ptr<SmppCommand> resp = SmppCommand::mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::MSGQFUL);
        ct.putCommand(resp);
    } else {
        if(entPtr->info.inQueueLimit>0 && entPtr->getQueueCount() >= entPtr->info.inQueueLimit) {
            smsc_log_warn(limitsLog,"Denied submit from '%s' by inQueueLimit:%d/%d",entPtr->info.systemId.c_str(),entPtr->getQueueCount(),entPtr->info.inQueueLimit);
            std::auto_ptr<SmppCommand> resp = SmppCommand::mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::MSGQFUL);
            ct.putCommand(resp);
        } else {
            queue.Push( cmd.release() );
            queueCount->setValue(queue.Count());
            entPtr->incCnt->increment();
            entPtr->incQueueCount();
            licenseCounter->increment();
        }
    }
    queueMon.notify();
}

void SmppManagerImpl::sendReceipt(Address& from, Address& to, int state, const char* msgId, const char* dst_sme_id, uint32_t netErrCode)
{
    SMS sms;
    if (msgId && msgId[0]) sms.setStrProperty(smsc::sms::Tag::SMPP_RECEIPTED_MESSAGE_ID, msgId);
    else {
    smsc_log_warn(log, "MSAG Receipt: MsgId is NULL! from=%s, to=%s, state=%d, dst_sme_id=%s",
              from.toString().c_str(), to.toString().c_str(), state, dst_sme_id);
    //abort(); // was todo: Remove it! For testing purposes only.
    return; // Do not send receipt at all (msgId missed)
    }
    sms.setOriginatingAddress(from);
    sms.setDestinationAddress(to);
    sms.setIntProperty(smsc::sms::Tag::SMPP_MSG_STATE, state);
    sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS, 0x4);
    if(netErrCode)
    {
        sms.setIntProperty(smsc::sms::Tag::SMPP_NETWORK_ERROR_CODE, netErrCode);
    }

    std::auto_ptr<SmppCommand> cmd = SmppCommand::makeDeliverySm(sms, 0);
    cmd->setFlag(SmppCommandFlags::NOTIFICATION_RECEIPT);
    {
        MutexGuard regmg(regMtx);
        SmppEntity **ptr=registry.GetPtr(dst_sme_id);
        if(!ptr)throw Exception("Unknown dst system id:%s", dst_sme_id);
        cmd->setDstEntity(*ptr);
    }

    smsc_log_debug(log, "MSAG Receipt: Sent from=%s, to=%s, state=%d, msgId=%s, dst_sme_id=%s",
                   from.toString().c_str(), to.toString().c_str(), state, msgId, dst_sme_id);
    MutexGuard mg(queueMon);
    queue.Push(cmd.release());
    queueCount->setValue(queue.Count());
    queueMon.notify();
}

bool SmppManagerImpl::getCommand(SmppCommand*& cmd)
{
    MutexGuard mg(queueMon);

    while((running || lcmProcessingCount || queuedCmdCount) && !queue.Count() && !lcmQueue.Count() && !respQueue.Count())
    {
        queueMon.wait(5000);

        time_t now=time(NULL);
        if(now - lastExpireProcess > 5)
        {
            lastExpireProcess=now;
            cmd = SmppCommand::makeCommand(PROCESSEXPIREDRESP,0,0,0).release();
            return true;
        }

        if(now-lastLicenseExpTest>10*60)
        {
            if(now>ConfigManager::Instance().getLicense().expdate)
            {
                smsc_log_error(log,"License expired");
                kill(getpid(),SIGTERM);
                return false;
            }
            struct tm ltm;
            localtime_r(&now,&ltm);
            if(ltm.tm_hour!=licenseFileCheckHour)
            {
                ConfigManager::Instance().checkLicenseFile();
                licenseFileCheckHour=ltm.tm_hour;
            }
            lastLicenseExpTest=now;
        }
    }

    time_t now=time(NULL);
    if(now - lastExpireProcess > 5)
    {
        lastExpireProcess=now;
        cmd = SmppCommand::makeCommand(PROCESSEXPIREDRESP,0,0,0).release();
        return true;
    }

//  if(!queue.Count() && !lcmQueue.Count() && !respQueue.Count())
//    return false;

    if(lcmQueue.Count())
    {
        lcmQueue.Pop(cmd);
    }
    else if(respQueue.Count())
    {
        respQueue.Pop(cmd);
    }
    else if(queue.Count())
    {
        queue.Pop(cmd);
        queueCount->setValue(queue.Count());
        if (cmd->getEntity()) {
            cmd->getEntity()->decQueueCount();
        }
    }
    else
    {
        queueCount->setValue(0);
        return false;
    }
    return true;
}

void SmppManagerImpl::continueExecution( LongCallContextBase* lcmCtx, bool dropped )
{
    std::auto_ptr<SmppCommand> cx
        (reinterpret_cast<SmppCommand*>(lcmCtx->stateMachineContext));
    lcmCtx->stateMachineContext = 0;
    lcmCtx->continueExec = ! dropped;

    Session* session = cx->getSession();
    __require__( session );
    smsc_log_debug( log, "continueExec(cmd=%p,sess=%p/%s,drop=%d)", cx.get(),
                    session, session->sessionKey().toString().c_str(),
                    dropped ? 1 : 0 );
    if ( dropped ) {
        // we need to get session here to force unlocking
        ActiveSession as = sessions::SessionManager::Instance()
            .getSession( session->sessionKey(), cx, false );
        __require__( as.get() );
    }

    MutexGuard mg(queueMon);
    lcmProcessingCount--;

    if (!dropped)
    {
        lcmQueue.Push( cx.release() );
        queueMon.notify();
    }
}


bool SmppManagerImpl::makeLongCall( std::auto_ptr<SmppCommand>& cx, ActiveSession& session )
{
    SmppCommand* cmd = cx.get();
    LongCallContext& lcmCtx = session->getLongCallContext();
    lcmCtx.stateMachineContext = cx.release();
    lcmCtx.initiator = this;
    // cmd->setSession(session);

    bool b = LongCallManager::Instance().call( &lcmCtx );
    if (b) {
        session.leaveLocked();
        MutexGuard mg(queueMon);
        lcmProcessingCount++;
    } else {
        cx.reset( cmd );
        lcmCtx.stateMachineContext = 0;
    }
    return b;
}


void SmppManagerImpl::pushCommand( SmppCommand* cmd )
{
    if ( ! cmd ) {
        smsc_log_error(log,"cmd = (null) is passed to smppmanager::pushCommand");
        ::abort();
        return;
    }

    MutexGuard mg(queueMon);
    lcmQueue.Push( cmd );
    queueMon.notify();
}

unsigned SmppManagerImpl::pushSessionCommand( SmppCommand* cmd, int action )
{
    if ( ! cmd ) {
        smsc_log_error(log,"cmd = (null) is passed to smppmanager::pushSessionCommand");
        ::abort();
        return unsigned(-1);
    }

    MutexGuard mg(queueMon);
    if ( action == SCAGCommandQueue::RESERVE ) {
        ++queuedCmdCount;
        smsc_log_debug( log, "reserve place for a cmd=%p: respQsz=%u, Qsz=%u, lcmCount=%u", cmd, respQueue.Count(), queue.Count(), lcmProcessingCount );
        // FIXME: should we return -1 when stopped ?
        return queuedCmdCount;
    }

    if ( action == SCAGCommandQueue::PUSH ) {
        smsc_log_error( log, "SmppManager::pushSessionCommand is intended for session queue commands, so RESERVE/MOVE should be used");
        throw SCAGException( "SmppManager::pushSessionCommand is intended for session queue commands, so RESERVE/MOVE should be used" );
    }

    assert( action == SCAGCommandQueue::MOVE );

    // action MOVE
    if ( queuedCmdCount > 0 ) --queuedCmdCount;
    if ( cmd->isResp() ) {
        respQueue.Push( cmd );
    } else {
        queue.Push( cmd );
        queueCount->setValue(queue.Count());
        // for being decremented in getCommand
        if ( cmd->getEntity() ) cmd->getEntity()->incQueueCount();
    }
    smsc_log_debug( log, "reserved cmd=%p moved onto queue: respQsz=%u, Qsz=%u, queuedCount=%u",
                    cmd, respQueue.Count(), queue.Count(), queuedCmdCount );
    queueMon.notify();
    return queuedCmdCount;
}


void SmppManagerImpl::reloadTestRoutes(const RouteConfig& rcfg,
                                       std::vector<std::string>* traceit )
{
    auto_ptr<router::RouteManager> router(new router::RouteManager());
    loadRoutes(router.get(),rcfg,traceit);
  ResetTestRouteManager(router.release());
}

util::RefferGuard<router::RouteManager> SmppManagerImpl::getTestRouterInstance()
{
    MutexGuard g(routerSwitchMutex);
    return util::RefferGuard<router::RouteManager>(testRouter_);
}

void SmppManagerImpl::ResetTestRouteManager(router::RouteManager* manager)
{
    MutexGuard g(routerSwitchMutex);
    if ( testRouter_ ) testRouter_->Release();
    testRouter_ = new util::Reffer<router::RouteManager>(manager);
}

void SmppManagerImpl::getQueueLen(uint32_t& reqQueueLen, uint32_t& respQueueLen, uint32_t& lcmQueueLen)
{
    reqQueueLen = queue.Count();
    respQueueLen = respQueue.Count();
    lcmQueueLen = lcmQueue.Count();
}


unsigned SmppManagerImpl::getUnsigned( const char* name, unsigned defval ) const
{
    try {
        int val = ConfigManager::Instance().getConfig()->getInt( name );
        if ( val < 0 ) throw SCAGException( "parameter %s should not be negative", name );
        return unsigned(val);
    } catch ( std::exception& e ) {
        smsc_log_warn( log, "exception on config param %s (%s), using %u", name, e.what(), defval );
    } catch (...) {
        smsc_log_warn( log, "unknown exception on config param %s, using %u", name, defval );
    }
    return defval;
}

}//smpp
}//transport
}//scag

