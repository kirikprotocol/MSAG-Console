#include <ctype.h>
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
    SmppEntity** ptr=registry.GetPtr(info.smeSystemId);
    return ptr ? *ptr : 0;
  }

  virtual SmppEntity* getSmppEntity(const char* systemId)const
  {
    MutexGuard mg(regMtx);
    SmppEntity** ptr=registry.GetPtr(systemId);
    return ptr ? *ptr : 0;
  }

protected:
  smsc::logger::Logger* log;
  smsc::logger::Logger* limitsLog;
  buf::Hash<SmppEntity*> registry;
  mutable sync::Mutex regMtx;
  SmppSocketManager sm;

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
};

bool SmppManager::inited = false;
Mutex SmppManager::initLock;

inline unsigned GetLongevity(SmppManager*) { return 5; }
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
TAGDEF(outQueueLimit)
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

template <int SZ>
void FillStringValue(DOMNamedNodeMap* attr,buf::FixedLengthString<SZ>& str)
{
  XmlStr value(attr->getNamedItem(XmlStr("value"))->getNodeValue());
  str=value.c_str();
}

template <int SZ>
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
  int len=strlen(bindTypeName);
  for(int i=0;i<len;i++)bindTypeName[i]=tolower(bindTypeName[i]);
  if     (!strcmp(bindTypeName,"rx")) return btReceiver;
  else if(!strcmp(bindTypeName,"tx")) return btTransmitter;
  else if(!strcmp(bindTypeName,"trx"))return btTransceiver;
  throw smsc::util::Exception("Unknown bind mode:%s",bindTypeName.c_str());
}

bool GetBoolValue(DOMNamedNodeMap* attr)
{
  char bindTypeName[16];
  FillStringValue(attr,bindTypeName);
  int len=strlen(bindTypeName);
  for(int i=0;i<len;i++)bindTypeName[i]=tolower(bindTypeName[i]);
  if     (!strcmp(bindTypeName,"true")) return true;
  else if(!strcmp(bindTypeName,"false")) return false;
  throw smsc::util::Exception("Invalid value for param 'enabled':%s",bindTypeName);
}

static void ParseTag(SmppManagerImpl* smppMan,DOMNodeList* list,SmppEntityType et)
{
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("smppMan");
  using namespace smsc::util::xml;
  unsigned listLength = list->getLength();
  SmppEntityInfo entity;
  entity.type=et;
  for (unsigned i=0; i<listLength; i++)
  {
    bool enabled=false;
    DOMNode *record = list->item(i);
    //DOMNodeList *children = record->getChildNodes();
    //unsigned childrenCount= children->getLength();
    //for (unsigned j=0; j<childrenCount; j++)
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
      if(et==etService && *tagPtr>tag_smscParams)
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
          enabled=GetBoolValue(attrs);
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
      }
    }
    if(enabled)
    {
      smppMan->addSmppEntity(entity);
    }else
    {
      smsc_log_info(log,"Record with systemId='%s' disabled and skipped",entity.systemId.c_str());
    }
  }
}

SmppManagerImpl::SmppManagerImpl():sm(this,this), ConfigListener(SMPPMAN_CFG), testRouter_(0)
{
  log=smsc::logger::Logger::getInstance("smppMan");
  limitsLog=smsc::logger::Logger::getInstance("smpp.lmt");
  running=false;
  lastUid=0;
  lastExpireProcess=0;
  lcmProcessingCount = 0;
}

SmppManagerImpl::~SmppManagerImpl()
{
    StopProcessing();
}

void SmppManagerImpl::Init(const char* cfgFile)
{
  using namespace smsc::util::xml;
  DOMTreeReader reader;
  DOMDocument* doc=reader.read(cfgFile);
  DOMElement *elem = doc->getDocumentElement();
  DOMNodeList *list = elem->getElementsByTagName(XmlStr("smerecord"));

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
    int n=0,i=0;
    int len=strlen(tags);
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
  SmppEntity** ptr=registry.GetPtr(info.systemId);
  if(ptr)
  {
    if((**ptr).info.type!=etUnknown)
    {
      throw smsc::util::Exception("Duplicate systemId='%s'",info.systemId.c_str());
    }
    (**ptr).info=info;
  }else
  {
    registry.Insert(info.systemId,new SmppEntity(info));
  }
  if(info.type==etSmsc)
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
  SmppEntity** ptr=registry.GetPtr(info.systemId);
  if(!ptr)
  {
    throw smsc::util::Exception("updateSmppEntity:Enitity with systemId='%s' not found",info.systemId.c_str());
  }
  SmppEntity& ent=**ptr;
  MutexGuard emg(ent.mtx);
  ent.info=info;
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

    sm.getSmscConnectorAdmin()->updateSmscConnect(ci);
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
        smsc_log_info(limitsLog,"Denied %s from '%s' due to shutting down", i == DELIVERY ? "DELIVERY" : (i == SUBMIT ? "SUBMIT" : "DATASM"), entPtr->info.systemId.c_str());
        SmppCommand resp=SmppCommand::makeSubmitSmResp("",cmd->get_dialogId(),smsc::system::Status::SYSERR);
        ct->putCommand(resp);
        return;
    }
    int cnt=entPtr->incCnt.Get();
    if(entPtr->info.sendLimit>0 && cnt/5>=entPtr->info.sendLimit && cmd.getCommandId()==SUBMIT && !cmd->get_sms()->hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP))
    {
      smsc_log_info(limitsLog,"Denied submit from '%s' by sendLimit:%d/%d",entPtr->info.systemId.c_str(),cnt/5,entPtr->info.sendLimit);
      SmppCommand resp=SmppCommand::makeSubmitSmResp("",cmd->get_dialogId(),smsc::system::Status::MSGQFUL);
      ct->putCommand(resp);
    }else if(queue.Count()>=queueLimit && !cmd->get_sms()->hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP))
    {
      smsc_log_info(limitsLog,"Denied submit from '%s' by queueLimit:%d/%d",entPtr->info.systemId.c_str(),queue.Count(),queueLimit);
      SmppCommand resp=SmppCommand::makeSubmitSmResp("",cmd->get_dialogId(),smsc::system::Status::MSGQFUL);
      ct->putCommand(resp);
    }else
    {
      if(entPtr->info.inQueueLimit>0 && entPtr->getQueueCount()>=entPtr->info.inQueueLimit)
      {
        smsc_log_info(limitsLog,"Denied submit from '%s' by inQueueLimit:%d/%d",entPtr->info.systemId.c_str(),entPtr->getQueueCount(),entPtr->info.inQueueLimit);
        SmppCommand resp=SmppCommand::makeSubmitSmResp("",cmd->get_dialogId(),smsc::system::Status::MSGQFUL);
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
    sms.setIntProperty(Tag::SMPP_NETWORK_ERROR_CODE, netErrCode);    

    SmppCommand& cmd = SmppCommand::makeDeliverySm(sms, 0);
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

