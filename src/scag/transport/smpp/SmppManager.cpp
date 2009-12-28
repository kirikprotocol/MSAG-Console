#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "SmppManager.h"
#include "util/xml/DOMTreeReader.h"
#include "util/xml/utilFunctions.h"
#include "core/buffers/Hash.hpp"
#include "SmppEntity.h"
#include "scag/config/route/RouteConfig.h"
#include "router/load_routes.h"
#include "scag/config/ConfigManager.h"
#include "SmppStateMachine.h"
#include "scag/exc/SCAGExceptions.h"
#include <scag/util/singleton/Singleton.h>
#include <map>
#include <list>

namespace scag{
namespace transport{
namespace smpp{

using namespace xercesc;
using namespace smsc::util::xml;
using namespace scag::exceptions;
using namespace scag::util::singleton;

class SmppManagerImpl: public SmppManager, public ConfigListener
{
public:
  SmppManagerImpl();
  ~SmppManagerImpl();
  void Init(const char* cfgFile);
  void LoadRoutes(const char* cfgFile);
  void ReloadRoutes();

  //admin
  virtual void addSmppEntity(const SmppEntityInfo& info);
  virtual void updateSmppEntity(const SmppEntityInfo& info);
  virtual void disconnectSmppEntity(const char* sysId);
  virtual void deleteSmppEntity(const char* sysId);
  virtual SmppEntityAdminInfoList * getEntityAdminInfoList(SmppEntityType entType);

  //registragor
  virtual int registerSmeChannel(const char* sysId,const char* pwd,SmppBindType bt,SmppChannel* ch);
  virtual int registerSmscChannel(SmppChannel* ch);
  virtual void unregisterChannel(SmppChannel* ch);

  //queue
  virtual void putCommand(SmppChannel* ct,SmppCommand& cmd);
  virtual bool getCommand(SmppCommand& cmd);

  virtual void continueExecution(LongCallContext* lcmCtx, bool dropped);
  virtual bool makeLongCall(SmppCommand& cx, SessionPtr& session);

  //manager
  void  sendReceipt(Address& from, Address& to, int state, const char* msgId, const char* dst_sme_id, uint32_t netErrCode);
  virtual void pushCommand(SmppCommand& cmd);

  void configChanged();

  void reloadTestRoutes(const RouteConfig& rcfg);
  RefferGuard<RouteManager> getTestRouterInstance();
  void ResetTestRouteManager(RouteManager* manager);

  void getQueueLen(uint32_t& reqQueueLen, uint32_t& respQueueLen, uint32_t& lcmQueueLen);

  void StopProcessing()
  {
    {
        sync::MutexGuard mg(queueMon);
        if(!running) return;
        running=false;
        queueMon.notifyAll();
    }
    smsc_log_debug(log, "SmppManager shutting down");
    tp.shutdown(0);
    sm.shutdown();
    smsc_log_debug(log, "SmppManager shutdown");
  }

  //SmppRouter
  virtual SmppEntity* RouteSms(router::SmeIndex srcidx, const smsc::sms::Address& source, const smsc::sms::Address& dest, router::RouteInfo& info)
  {
    {
      RouterRef ref;
      {
        sync::MutexGuard rsmg(routerSwitchMtx);
        ref=routeMan;
      }
      if(!ref->lookup(srcidx,source,dest,info))return 0;
    }
    MutexGuard mg(regMtx);
    SmppEntity** ptr=registry.GetPtr(info.smeSystemId.c_str());
    if(ptr)
    {
      if((*ptr)->info.enabled)
      {
        return *ptr;
      }else
      {
        return 0;
      }
    }
    MetaEntity** pme=metaRegistry.GetPtr(info.smeSystemId.c_str());
    if(!pme)return 0;
    MetaEntity& me=**pme;
    SmppEntity* rv=0;
    if(me.info.persistanceEnabled)
    {
      if(me.info.type==mtMetaService)
      {
        rv=me.getEntity(source);
      }else
      {
        rv=me.getEntity(dest);
      }
    }else
    {
      rv=me.getEntity();
    }
    if(rv)
    {
      info.smeSystemId=rv->getSystemId();
    }
    return rv;
  }

  virtual SmppEntity* getSmppEntity(const char* systemId) const
  {
    MutexGuard mg(regMtx);
    SmppEntity* const *ptr=registry.GetPtr(systemId);
    return ptr ? (*ptr)->info.enabled?*ptr:0 : 0;
  }

  void addMetaEntity(MetaEntityInfo info)
  {
    MutexGuard mg(regMtx);
    if(metaRegistry.Exists(info.systemId.c_str()))
    {
      throw Exception("Duplicate meta entity:%s",info.systemId.c_str());
    }
    MetaEntity* ptr=new MetaEntity;
    ptr->info=info;
    metaRegistry.Insert(info.systemId.c_str(),ptr);
  }

  void updateMetaEntity(MetaEntityInfo info)
  {
    MutexGuard mg(regMtx);
    MetaEntity** ptr=metaRegistry.GetPtr(info.systemId.c_str());
    if(!ptr)
    {
      throw Exception("updateMetaEntity::Meta entity not found:%s",info.systemId.c_str());
    }
    (*ptr)->info=info;
  }

  void deleteMetaEntity(const char* id)
  {
    MutexGuard mg(regMtx);
    MetaEntity** ptr=metaRegistry.GetPtr(id);
    if(!ptr)
    {
      throw Exception("deleteMetaEntity::Meta entity not found:%s",id);
    }
    if(!(*ptr)->ents.empty())
    {
      throw Exception("deleteMetaEntity::Meta entity isn't empty:%s",id);
    }
    metaRegistry.Delete(id);
  }

  void addMetaEndPoint(const char* metaId,const char* sysId)
  {
    MutexGuard mg(regMtx);
    MetaEntity** mptr=metaRegistry.GetPtr(metaId);
    if(!mptr)
    {
      throw Exception("addMetaEndPoint::Meta entity not found:%s",metaId);
    }
    SmppEntity** sptr=registry.GetPtr(sysId);
    if(!sptr)
    {
      throw Exception("addMetaEndPoint::Smpp entity not found:%s",sysId);
    }
    if(((*mptr)->info.type==mtMetaService && (*sptr)->info.type!=etService)||
       ((*mptr)->info.type==mtMetaSmsc && (*sptr)->info.type!=etSmsc))
    {
      throw Exception("addMetaEndPoint::incompatible type '%s' and '%s'",metaId,sysId);
    }
    MetaEntity &me=**mptr;
    MetaEntity::MetaEntsVector::iterator it;
    for(it=me.ents.begin();it!=me.ents.end();it++)
    {
      if(it->ptr==*sptr)
      {
        throw Exception("addMetaEndPoint::duplicate entity '%s' in meta entity '%s'",sysId,metaId);
      }
    }
    MetaEntity::MetaInfo mi;
    mi.ptr=*sptr;
    (*mptr)->ents.push_back(mi);
  }

  void removeMetaEndPoint(const char* metaId,const char* sysId)
  {
    MutexGuard mg(regMtx);
    MetaEntity** mptr=metaRegistry.GetPtr(metaId);
    if(!mptr)
    {
      throw Exception("removeMetaEndPoint::Meta entity not found:%s",metaId);
    }
    SmppEntity** sptr=registry.GetPtr(sysId);
    if(!sptr)
    {
      throw Exception("removeMetaEndPoint::Smpp entity not found:%s",sysId);
    }
    MetaEntity& me=**mptr;
    MetaEntity::MetaEntsVector::iterator it;
    for(it=me.ents.begin();it!=me.ents.end();it++)
    {
      if(it->ptr==*sptr)
      {
        me.ents.erase(it);
        return;
      }
    }
    throw Exception("removeMetaEndPoint::unexpected error - smpp entity not found in meta entity :(");
  }

  bool LoadEntityFromConfig(SmppEntityInfo& info,const char* sysId,SmppEntityType et);
  bool LoadMetaEntityFromConfig(MetaEntityInfo& info,const char* sysId);


protected:

  struct MetaEntity{
    struct MetaInfo{
      SmppEntity* ptr;
    };
    MetaEntity():lastEntity(0)
    {
      seed=(unsigned int)time(NULL);
    }
    MetaEntityInfo info;
    typedef std::vector<MetaInfo> MetaEntsVector;
    MetaEntsVector ents;
    int lastEntity;
    unsigned int seed;
    sync::Mutex mtx;

    static time_t expirationTimeout;

  private:
      struct MappingValue;
      typedef std::list< MappingValue >                   ValueList;
      typedef std::map< Address, ValueList::iterator >    AbonentsMap;      
      typedef std::multimap<time_t, ValueList::iterator > TimeoutsMap;

      struct MappingValue
      {
          MappingValue() : ptr(0) {}
          MappingValue(SmppEntity* arg) : ptr(arg) {}

          SmppEntity* ptr;
          AbonentsMap::iterator aiter;
          TimeoutsMap::iterator titer;
      };

      ValueList   valueList;
      AbonentsMap storedMappings;
      TimeoutsMap timeMap;

    SmppEntity* getStoredMapping(const Address& addr)
    {
      AbonentsMap::const_iterator it=storedMappings.find(addr);
      if(it==storedMappings.end())return 0;
      ValueList::iterator vit = it->second;
      timeMap.erase(vit->titer);
      vit->titer = timeMap.insert(TimeoutsMap::value_type(time(NULL)+expirationTimeout,vit));
      return vit->ptr;
    }

    void createStoredMapping(const Address& addr,SmppEntity* ptr)
    {
        ValueList::iterator vit = valueList.insert(valueList.begin(),MappingValue(ptr));
        vit->aiter = storedMappings.insert(AbonentsMap::value_type(addr,vit)).first;
        vit->titer = timeMap.insert(TimeoutsMap::value_type(time(NULL)+expirationTimeout,vit));
    }

    void expireMappings()
    {
      time_t now=time(NULL);
      TimeoutsMap::iterator it;
      while(!timeMap.empty() && now>(it=timeMap.begin())->first)
      {
          ValueList::iterator vit = it->second;
          storedMappings.erase(vit->aiter);
          valueList.erase(vit);
          timeMap.erase(it);
      }
    }
      
  public:

    SmppEntity* getEntity(const Address& addr)
    {
      sync::MutexGuard mg(mtx);
      SmppEntity* ptr=getStoredMapping(addr);
      if(!ptr)
      {
        mtx.Unlock();
        try{
          ptr=getEntity();
          createStoredMapping(addr,ptr);
        }catch(...)
        {

        }
        mtx.Lock();
      }
      expireMappings();
      return ptr;
    }

    SmppEntity* getEntity()
    {
      sync::MutexGuard mg(mtx);
      if(info.policy==bpRandom)
      {
        double val=rand_r(&seed);
        val/=RAND_MAX;
        lastEntity=val*ents.size();
      }
      for(int i=0;i<ents.size();i++)
      {
        lastEntity++;
        if(lastEntity>=ents.size())
        {
          lastEntity=0;
        }
        sync::MutexGuard mg2(ents[lastEntity].ptr->mtx);
        if(ents[lastEntity].ptr->bt==btNone || !ents[lastEntity].ptr->info.enabled)continue;
        return ents[lastEntity].ptr;
      }
      return 0;
    }
  };

  smsc::logger::Logger* log;
  smsc::logger::Logger* limitsLog;
  buf::Hash<SmppEntity*> registry;
  buf::Hash<MetaEntity*> metaRegistry;
  mutable sync::Mutex regMtx;
  SmppSocketManager sm;

  smsc::util::TimeSlotCounter<> licenseCounter;
  time_t lastLicenseExpTest;
  int licenseFileCheckHour;


  bool running;

  buf::CyclicQueue<SmppCommand> queue, lcmQueue, respQueue;
  sync::EventMonitor queueMon;
  time_t lastExpireProcess;

  typedef RefPtr<router::RouteManager,sync::Mutex> RouterRef;
  RouterRef routeMan;
  sync::Mutex routerSwitchMtx;
//  std::string routerConfigFile;

  Mutex routerSwitchMutex;
  Reffer<RouteManager>* testRouter_;


  thr::ThreadPool tp;

  int lastUid;
  int queueLimit;

  uint32_t lcmProcessingCount;
  std::string cfgFileName;
};

time_t SmppManagerImpl::MetaEntity::expirationTimeout=300;


bool SmppManager::inited = false;
Mutex SmppManager::initLock;

inline unsigned GetLongevity(SmppManager*) { return 251; }
typedef SingletonHolder<SmppManagerImpl> SingleSM;

SmppManager& SmppManager::Instance()
{
    if (!inited)
    {
        MutexGuard guard(initLock);
        if (!inited)
            throw std::runtime_error("SmppManager not inited!");
    }
    return SingleSM::Instance();
}

void SmppManager::Init(const char* cfg)
{
    if (!inited)
    {
        MutexGuard guard(initLock);
        if(!inited) {
            SmppManagerImpl& sm = SingleSM::Instance();
            sm.Init(cfg);
            inited = true;
        }
    }
}

void SmppManager::shutdown()
{
    if (!inited)
    {
        MutexGuard guard(initLock);
        if (!inited)
            throw std::runtime_error("SmppManager not inited!");
    }
    SingleSM::Instance().StopProcessing();
}

const int tag_smscParams=1024;

enum ParamTag{
tag_systemId,
tag_password,
tag_timeout,
tag_mode,
tag_enabled,
tag_providerId,
tag_maxSmsPerSec,
tag_metaGroup,
tag_persistance,
tag_inQueueLimit,
tag_outQueueLimit,
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
TAGDEF(uid),
TAGDEF(bindSystemId),
TAGDEF(bindPassword),
TAGDEF(addressRange),
TAGDEF(systemType),
TAGDEF(inQueueLimit),
TAGDEF(outQueueLimit),
TAGDEF(metaGroup),
TAGDEF(persistance)
};

struct ParamsHash:public smsc::core::buffers::Hash<ParamTag>{
  ParamsHash()
  {
    for(int i=0;i<sizeof(params)/sizeof(params[0]);i++)
    {
      Insert(params[i].name,params[i].tag);
    }
  }
};

static const ParamsHash paramsHash;

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
  throw smsc::util::Exception("Invalid value for param 'enabled':%s",bindTypeName);
}

static void FillEntity(SmppEntityInfo& entity,DOMNode* record)
{
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("smppMan");
  for(DOMNode *node = record->getFirstChild(); node != 0; node = node->getNextSibling())
  {
    //DOMNode *node = children->item(i);
    if (node->getNodeType() != DOMNode::ELEMENT_NODE)continue;

    DOMNamedNodeMap *attrs = node->getAttributes();
    if(!attrs)
    {
      throw smsc::util::Exception("Invalid smpp.xml configuration file");
    }
    XmlStr paramName(attrs->getNamedItem(XmlStr("name"))->getNodeValue());
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
    XmlStr paramName(attrs->getNamedItem(XmlStr("name"))->getNodeValue());
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


SmppManagerImpl::SmppManagerImpl():sm(this,this), ConfigListener(SMPPMAN_CFG), testRouter_(0),licenseCounter(10,20)
{
  log=smsc::logger::Logger::getInstance("smppMan");
  limitsLog=smsc::logger::Logger::getInstance("smpp.lmt");
  running=false;
  lastUid=0;
  lastExpireProcess=0;
  lcmProcessingCount = 0;
  lastLicenseExpTest=0;
  licenseFileCheckHour=0;
}

SmppManagerImpl::~SmppManagerImpl()
{
    StopProcessing();
}

void SmppManagerImpl::Init(const char* cfgFile)
{
  cfgFileName=cfgFile;
  using namespace smsc::util::xml;
  DOMTreeReader reader;
  DOMDocument* doc=reader.read(cfgFile);
  DOMElement *elem = doc->getDocumentElement();

  DOMNodeList *list = elem->getElementsByTagName(XmlStr("metasmerecord"));
  ParseMetaTag(this,list,mtMetaService);

  list = elem->getElementsByTagName(XmlStr("metasmscrecord"));
  ParseMetaTag(this,list,mtMetaSmsc);


  list = elem->getElementsByTagName(XmlStr("smerecord"));
  ParseTag(this,list,etService);

  list = elem->getElementsByTagName(XmlStr("smscrecord"));
  ParseTag(this,list,etSmsc);

  LoadRoutes("conf/smpp_routes.xml");
  running=true;

  int stmCnt = 0;
  try
  {
      stmCnt = scag::config::ConfigManager::Instance().getConfig()->getInt("smpp.core.state_machines_count");
  } catch (HashInvalidKeyException& e)
  {
      running=false;
      throw SCAGException("Invalid parameter 'smpp.core.state_machines_count'");
  }

  queueLimit=1000;
  try{
    queueLimit= scag::config::ConfigManager::Instance().getConfig()->getInt("smpp.core.eventQueueLimit");
  }catch(HashInvalidKeyException& e)
  {
    smsc_log_warn(log,"smpp.queueLimit not found! Using default(%d)",queueLimit);
  }

  try{
    const char* tags=scag::config::ConfigManager::Instance().getConfig()->getString("smpp.transitOptionalTags");
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

  smsc_log_info(log,"Starting %d state machines",stmCnt);
  for(int i=0;i<stmCnt;i++)
  {
    tp.startTask(new StateMachine(this,this));
  }
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
  scag::config::RouteConfig& cfg = scag::config::ConfigManager::Instance().getRouteConfig();
  router::RouteManager* newman=new router::RouteManager();
  router::loadRoutes(newman,cfg,false);
  {
    sync::MutexGuard mg(routerSwitchMtx);
    routeMan=newman;
  }
}

void SmppManagerImpl::ReloadRoutes()
{
  scag::config::RouteConfig& cfg = scag::config::ConfigManager::Instance().getRouteConfig();
  router::RouteManager* newman=new router::RouteManager();
  router::loadRoutes(newman,cfg,false);
  {
    sync::MutexGuard mg(routerSwitchMtx);
    routeMan=newman;
  }
}

void SmppManagerImpl::addSmppEntity(const SmppEntityInfo& info)
{
  smsc_log_debug(log,"addSmppEntity:%s",info.systemId.c_str());
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr=registry.GetPtr(info.systemId.c_str());
  if(ptr)
  {
    if((**ptr).info.type!=etUnknown)
    {
      throw smsc::util::Exception("Duplicate systemId='%s'",info.systemId.c_str());
    }
    (**ptr).info=info;
  }else
  {
    registry.Insert(info.systemId.c_str(),new SmppEntity(info));
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
  smsc_log_debug(log,"updateSmppEntity:%s",info.systemId.c_str());
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr=registry.GetPtr(info.systemId.c_str());
  if(!ptr)
  {
    throw smsc::util::Exception("updateSmppEntity:Enitity with systemId='%s' not found",info.systemId.c_str());
  }
  SmppEntity& ent=**ptr;
  MutexGuard emg(ent.mtx);
  bool oldEnabled=ent.info.enabled;
  ent.info=info;

  if(ent.info.type==etSmsc)
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

    if(oldEnabled==false && ent.info.enabled==true)
    {
      sm.getSmscConnectorAdmin()->addSmscConnect(ci);
    }else if(oldEnabled==true && ent.info.enabled==false)
    {
      sm.getSmscConnectorAdmin()->deleteSmscConnect(info.systemId.c_str());
    }else if(oldEnabled==true && ent.info.enabled==true)
    {
      sm.getSmscConnectorAdmin()->updateSmscConnect(ci);
    }
  }

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
  }

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
  }

  if(ent.info.type==etSmsc)
      sm.getSmscConnectorAdmin()->reportSmscDisconnect(sysId);
}

void SmppManagerImpl::deleteSmppEntity(const char* sysId)
{
  smsc_log_debug(log,"deleteSmppEntity:%s",sysId);
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr=registry.GetPtr(sysId);
  if(!ptr)
  {
    throw smsc::util::Exception("deleteSmppEntity:Enitity with systemId='%s' not found",sysId);
  }
  SmppEntity& ent=**ptr;
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
  }

  if(ent.info.type==etSmsc)
  {
    sm.getSmscConnectorAdmin()->deleteSmscConnect(sysId);
  }

  ent.bt=btNone;
  ent.channel=0;
  ent.transChannel=0;
  ent.recvChannel=0;
  ent.seq=0;
  ent.info.type=etUnknown;
}

SmppEntityAdminInfoList * SmppManagerImpl::getEntityAdminInfoList(SmppEntityType entType)
{
    MutexGuard mg(regMtx);

    SmppEntity * value = 0;
    char * key = 0;

    SmppEntityAdminInfoList * result = new SmppEntityAdminInfoList;

    for (buf::Hash<SmppEntity *>::Iterator it = registry.getIterator(); it.Next(key, value);)
    {
        MutexGuard emg(value->mtx);
        /*SmppEntityType type;
        std::string host;
        int  port;
        bool connected;*/

        if (value->info.type == entType)
        {
            SmppEntityAdminInfo ai = {value->getSystemId(), value->bt, value->info.host, value->info.port, value->connected};
            result->push_back(ai);
        }
    }

    return result;
}



int SmppManagerImpl::registerSmeChannel(const char* sysId,const char* pwd,SmppBindType bt,SmppChannel* ch)
{
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr=registry.GetPtr(sysId);
  if(!ptr)
  {
    smsc_log_info(log,"Failed to register sme with sysId='%s' - Not found",sysId);
    return rarFailed;
  }
  if(!(*ptr)->info.enabled)
  {
    smsc_log_info(log,"Failed to register sme with sysId='%s' - disabled",sysId);
    return rarFailed;
  }
  SmppEntity& ent=**ptr;
  if(!(ent.info.password == pwd))
  {
    smsc_log_info(log,"Failed to register sme with sysId='%s' - password mismatch:'%s'!='%s'",sysId,ent.info.password.c_str(),pwd);
    return rarFailed;
  }
  {
    MutexGuard mg2(ent.mtx);
    if(ent.info.type==etUnknown)
    {
      smsc_log_warn(log,"Failed to register sme with sysId='%s' - Entity deleted",sysId);
      return rarFailed;
    }
    if(ent.info.type!=etService)
    {
      smsc_log_warn(log,"Failed to register sme with sysId='%s' - Entity type is not sme",sysId);
      return rarFailed;
    }

    if((ent.info.bindType==btReceiver && bt!=btReceiver)||
       (ent.info.bindType==btTransmitter && bt!=btTransmitter))
    {
      smsc_log_info(log,"Failed to register sme with sysId='%s' - Invalid bind type",sysId);
      return rarFailed;
    }
    if((ent.bt==btReceiver && bt==btReceiver)||
       (ent.bt==btTransmitter && bt==btTransmitter)||
       (ent.bt!=btNone))
    {
      smsc_log_info(log,"Failed to register sme with sysId='%s' - Already registered",sysId);
      return rarAlready;
    }
    if(ent.bt==btNone)
    {
      ent.bt=bt;
    }else
    {
      ent.bt=btRecvAndTrans;
    }
    if(bt==btTransceiver)
    {
      ent.channel=ch;
    }else if(bt==btReceiver)
    {
      ent.recvChannel=ch;
    }else if(bt==btTransmitter)
    {
      ent.transChannel=ch;
    }
  }
  ent.info.host = ch->getPeer();
  ent.connected = true;
  ent.setUid(++lastUid);
  smsc_log_info(log,"Registered sme with sysId='%s'",sysId);
  return rarOk;
}

int SmppManagerImpl::registerSmscChannel(SmppChannel* ch)
{
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr=registry.GetPtr(ch->getSystemId());
  if(!ptr || (**ptr).info.bindType==btNone)
  {
    smsc_log_info(log,"Failed to register smsc with sysId='%s' - Not found",ch->getSystemId());
    return rarFailed;
  }
  if(!(**ptr).info.enabled)
  {
    smsc_log_info(log,"Failed to register smsc with sysId='%s' - Not found",ch->getSystemId());
    return rarFailed;
  }
  SmppEntity& ent=**ptr;
  {
    MutexGuard mg2(ent.mtx);
    ent.bt=btTransceiver;
    ent.channel=ch;
    smsc_log_info(log,"Registered smsc connection with sysId='%s'",ch->getSystemId());
  }
  ent.connected = true;
  ent.setUid(++lastUid);
  return rarOk;
}
void SmppManagerImpl::unregisterChannel(SmppChannel* ch)
{
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr=registry.GetPtr(ch->getSystemId());
  if(!ptr || (**ptr).bt==btNone)
  {
    smsc_log_info(log,"Failed to unregister smsc with sysId='%s' - Not found",ch->getSystemId());
    return;
  }
  SmppEntity& ent=**ptr;
  MutexGuard mg2(ent.mtx);
  if(ent.bt==btRecvAndTrans)
  {
    if(ch->getBindType()==btReceiver)
    {
      ent.bt=btTransmitter;
    }else if(ch->getBindType()==btTransmitter)
    {
      ent.bt=btReceiver;
    }else
    {
      smsc_log_warn(log,"Attempt to unregister channel with invalid bind type for sysId='%s'",ch->getSystemId());
    }
  }else
  {
    ent.bt=btNone;
  }
  ent.connected = false;
  if (ent.info.type == etService) ent.info.host = "";
}


static SmppCommand mkErrResp(int cmdId,int dlgId,int errCode)
{
  switch(cmdId)
  {
    case SUBMIT:return SmppCommand::makeSubmitSmResp("",dlgId,errCode);
    case DELIVERY:return SmppCommand::makeDeliverySmResp("",dlgId,errCode);
    case DATASM:return SmppCommand::makeDataSmResp("",dlgId,errCode);
    default:throw Exception("Unsupported commandId:%d",cmdId);
  }
}

void SmppManagerImpl::putCommand(SmppChannel* ct,SmppCommand& cmd)
{
  SmppEntity* entPtr=0;
  {
    MutexGuard regmg(regMtx);
    SmppEntity** ptr=registry.GetPtr(ct->getSystemId());
    if(!ptr)throw Exception("Unknown system id:%s",ct->getSystemId());
    cmd.setEntity(*ptr);
    entPtr=*ptr;
  }

  MutexGuard mg(queueMon);
  int i = cmd->get_commandId();
  if(i == DELIVERY_RESP || i == SUBMIT_RESP || i == DATASM_RESP)
    respQueue.Push(cmd);
  else
  {
    if(!running)
    {
        smsc_log_warn(limitsLog,"Denied %s from '%s' due to shutting down", i == DELIVERY ? "DELIVERY" : (i == SUBMIT ? "SUBMIT" : "DATASM"), entPtr->info.systemId.c_str());
        SmppCommand resp=mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::SYSERR);
        ct->putCommand(resp);
        return;
    }

    int licLimit=ConfigManager::Instance().getLicense().maxsms;
    int cntValue=licenseCounter.Get()/10;
    if(cntValue>licLimit)
    {
      bool allow=false;
      if(i==SUBMIT || i==DELIVERY || i==DATASM)
      {
        SMS& sms=*cmd->get_sms();
        if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
        {
          if(sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)!=USSD_PSSR_IND)
          {
            allow=true;
          }
        }
      }
      if(!allow)
      {
        smsc_log_info(limitsLog,"Denied by license limitation:%d/%d",cntValue,licLimit);
        SmppCommand resp=mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::THROTTLED);
        ct->putCommand(resp);
        return;
      }
    }

    int cnt=entPtr->incCnt.Get();
    if(entPtr->info.sendLimit>0 && cnt/5>=entPtr->info.sendLimit && cmd.getCommandId()==SUBMIT && !cmd->get_sms()->hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP))
    {
      smsc_log_warn(limitsLog,"Denied submit from '%s' by sendLimit:%d/%d",entPtr->info.systemId.c_str(),cnt/5,entPtr->info.sendLimit);
      SmppCommand resp=mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::MSGQFUL);
      ct->putCommand(resp);
    }else if(queue.Count()>=queueLimit && !cmd->get_sms()->hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP))
    {
      smsc_log_warn(limitsLog,"Denied submit from '%s' by queueLimit:%d/%d",entPtr->info.systemId.c_str(),queue.Count(),queueLimit);
      SmppCommand resp=mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::MSGQFUL);
      ct->putCommand(resp);
    }else
    {
      if(entPtr->info.inQueueLimit>0 && entPtr->getQueueCount()>=entPtr->info.inQueueLimit)
      {
        smsc_log_warn(limitsLog,"Denied submit from '%s' by inQueueLimit:%d/%d",entPtr->info.systemId.c_str(),entPtr->getQueueCount(),entPtr->info.inQueueLimit);
        SmppCommand resp=mkErrResp(i,cmd->get_dialogId(),smsc::system::Status::MSGQFUL);
        ct->putCommand(resp);
      }else
      {
        queue.Push(cmd);
        if(entPtr->info.sendLimit>0)
        {
          entPtr->incCnt.Inc();
          //smsc_log_debug(limitsLog,"cnt=%d",entPtr->incCnt.Get());
        }
        entPtr->incQueueCount();
        licenseCounter.Inc();
      }
    }
  }
  queueMon.notify();
}

void SmppManagerImpl::sendReceipt(Address& from, Address& to, int state, const char* msgId, const char* dst_sme_id, uint32_t netErrCode)
{
    SMS sms;
    if (msgId && msgId[0]) sms.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID, msgId);
    else {
    smsc_log_warn(log, "MSAG Receipt: MsgId is NULL! from=%s, to=%s, state=%d, dst_sme_id=%s",
              from.toString().c_str(), to.toString().c_str(), state, dst_sme_id);
    //abort(); // TODO: Remove it! For testing purposes only.
    return; // Do not send receipt at all (msgId missed)
    }
    sms.setOriginatingAddress(from);
    sms.setDestinationAddress(to);
    sms.setIntProperty(Tag::SMPP_MSG_STATE, state);
    sms.setIntProperty(Tag::SMPP_ESM_CLASS, 0x4);
    if(netErrCode)
    {
      sms.setIntProperty(Tag::SMPP_NETWORK_ERROR_CODE, netErrCode);
    }

    SmppCommand cmd = SmppCommand::makeDeliverySm(sms, 0);
    cmd->setFlag(SmppCommandFlags::NOTIFICATION_RECEIPT);
    {
        MutexGuard regmg(regMtx);
        SmppEntity **ptr=registry.GetPtr(dst_sme_id);
        if(!ptr)throw Exception("Unknown dst system id:%s", dst_sme_id);
        cmd.setDstEntity(*ptr);
    }

    smsc_log_debug(log, "MSAG Receipt: Sent from=%s, to=%s, state=%d, msgId=%s, dst_sme_id=%s",
           from.toString().c_str(), to.toString().c_str(), state, msgId, dst_sme_id);
    MutexGuard mg(queueMon);
    queue.Push(cmd);
    queueMon.notify();
}

bool SmppManagerImpl::getCommand(SmppCommand& cmd)
{
  MutexGuard mg(queueMon);

  while((running || lcmProcessingCount) && !queue.Count() && !lcmQueue.Count() && !respQueue.Count())
  {
    queueMon.wait(5000);

    time_t now=time(NULL);
    if(now - lastExpireProcess > 5)
    {
      lastExpireProcess=now;
      cmd=SmppCommand::makeCommand(PROCESSEXPIREDRESP,0,0,0);
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
    cmd=SmppCommand::makeCommand(PROCESSEXPIREDRESP,0,0,0);
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
    if(cmd.getEntity())cmd.getEntity()->decQueueCount();
  }
  else
  {
    return false;
  }
  return true;
}

void SmppManagerImpl::continueExecution(LongCallContext* lcmCtx, bool dropped)
{
    SmppCommand *cx = (SmppCommand*)lcmCtx->stateMachineContext;
    lcmCtx->continueExec = true;

    MutexGuard mg(queueMon);
    lcmProcessingCount--;
    if(!dropped)
    {
        lcmQueue.Push(*cx);
        queueMon.notify();
    }
    delete cx;
}

bool SmppManagerImpl::makeLongCall(SmppCommand& cx, SessionPtr& session)
{
    SmppCommand* cmd = new SmppCommand(cx);
    LongCallContext& lcmCtx = session->getLongCallContext();
    lcmCtx.stateMachineContext = cmd;
    lcmCtx.initiator = this;
    cmd->setSession(session);

    bool b = LongCallManager::Instance().call(&lcmCtx);
    MutexGuard mg(queueMon);
    if(b) lcmProcessingCount++;

    return b;
}

void SmppManagerImpl::pushCommand(SmppCommand& cmd)
{
    MutexGuard mg(queueMon);
    lcmQueue.Push(cmd);
    queueMon.notify();
}

void SmppManagerImpl::reloadTestRoutes(const RouteConfig& rcfg)
{
  auto_ptr<RouteManager> router(new RouteManager());
  loadRoutes(router.get(),rcfg,true);
  ResetTestRouteManager(router.release());
}

RefferGuard<RouteManager> SmppManagerImpl::getTestRouterInstance()
{
    MutexGuard g(routerSwitchMutex);
    return RefferGuard<RouteManager>(testRouter_);
}

void SmppManagerImpl::ResetTestRouteManager(RouteManager* manager)
{
    MutexGuard g(routerSwitchMutex);
    if ( testRouter_ ) testRouter_->Release();
    testRouter_ = new Reffer<RouteManager>(manager);
}

void SmppManagerImpl::getQueueLen(uint32_t& reqQueueLen, uint32_t& respQueueLen, uint32_t& lcmQueueLen)
{
    reqQueueLen = queue.Count();
    respQueueLen = respQueue.Count();
    lcmQueueLen = lcmQueue.Count();
}

}//smpp
}//transport
}//scag

