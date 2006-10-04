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

namespace scag{
namespace transport{
namespace smpp{

using namespace xercesc;
using namespace smsc::util::xml;
using namespace scag::exceptions;

enum ParamTag{
tag_systemId,
tag_password,
tag_timeout,
tag_mode,
tag_enabled,
tag_providerId,
tag_host=tag_providerId|1024,
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
TAGDEF(host),
TAGDEF(port),
TAGDEF(althost),
TAGDEF(altport),
TAGDEF(enabled),
TAGDEF(uid),
TAGDEF(bindSystemId),
TAGDEF(bindPassword),
TAGDEF(addressRange),
TAGDEF(systemType)
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

static void ParseTag(SmppManager* smppMan,DOMNodeList* list,SmppEntityType et)
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
      if(et==etService && *tagPtr>1024)
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

SmppManager::SmppManager():sm(this,this), ConfigListener(SMPPMAN_CFG)
{
  log=smsc::logger::Logger::getInstance("smppMan");
  running=false;
  lastUid=0;
  lastExpireProcess=0;
}


SmppManager::~SmppManager()
{
  StopProcessing();
  tp.shutdown();
  sm.shutdown();
}

void SmppManager::Init(const char* cfgFile)
{
  using namespace smsc::util::xml;
  DOMTreeReader reader;
  DOMDocument* doc=reader.read(cfgFile);
  DOMElement *elem = doc->getDocumentElement();
  DOMNodeList *list = elem->getElementsByTagName(XmlStr("smerecord"));

  ParseTag(this,list,etService);
  list = elem->getElementsByTagName(XmlStr("smscrecord"));

  ParseTag(this,list,etSmsc);

  LoadRoutes();
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

  smsc_log_info(log,"Starting %d state machines",stmCnt);
  for(int i=0;i<stmCnt;i++)
  {
    tp.startTask(new StateMachine(this,this));
  }
}

void SmppManager::configChanged()
{
}

void SmppManager::LoadRoutes()
{
  scag::config::RouteConfig& cfg = scag::config::ConfigManager::Instance().getRouteConfig();
  routeMan=new router::RouteManager();
  router::loadRoutes(routeMan.Get(),cfg,false);
}

void SmppManager::ReloadRoutes()
{
  scag::config::RouteConfig& cfg = scag::config::ConfigManager::Instance().getRouteConfig();
  RouterRef newRouter(new router::RouteManager());
  router::loadRoutes(newRouter.Get(),cfg);
  routeMan=newRouter;
}

void SmppManager::addSmppEntity(const SmppEntityInfo& info)
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

void SmppManager::updateSmppEntity(const SmppEntityInfo& info)
{
  smsc_log_debug(log,"updateSmppEntity:%s",info.systemId.c_str());
  sync::MutexGuard mg(regMtx);
  SmppEntity** ptr=registry.GetPtr(info.systemId);
  if(!ptr)
  {
    throw smsc::util::Exception("updateSmppEntity:Enitity with systemId='%s' not found",info.systemId.c_str());
  }
  (**ptr).info=info;
}

void SmppManager::deleteSmppEntity(const char* sysId)
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

SmppEntityAdminInfoList * SmppManager::getEntityAdminInfoList(SmppEntityType entType)
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
            SmppEntityAdminInfo ai = {value->getSystemId(), value->info.host, value->info.port, value->connected};
            result->push_back(ai);
        }
    }

    return result;
}



int SmppManager::registerSmeChannel(const char* sysId,const char* pwd,SmppBindType bt,SmppChannel* ch)
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
  ent.connected = true;
  ent.setUid(++lastUid);
  smsc_log_info(log,"Registered sme with sysId='%s'",sysId);
  return rarOk;
}

int SmppManager::registerSmscChannel(SmppChannel* ch)
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
void SmppManager::unregisterChannel(SmppChannel* ch)
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
}


void SmppManager::putCommand(SmppChannel* ct,SmppCommand& cmd)
{
  {
    MutexGuard regmg(regMtx);
    SmppEntity** ptr=registry.GetPtr(ct->getSystemId());
    if(!ptr)throw Exception("Unknown system id:%s",ct->getSystemId());
    cmd.setEntity(*ptr);
  }

  MutexGuard mg(queueMon);
  queue.Push(cmd);
  queueMon.notify();
}

bool SmppManager::getCommand(SmppCommand& cmd)
{
  MutexGuard mg(queueMon);
  while(running && queue.Count()==0)
  {
    queueMon.wait(5000);
    time_t now=time(NULL);
    if(now-lastExpireProcess>5)
    {
      lastExpireProcess=now;
      cmd=SmppCommand::makeCommand(PROCESSEXPIREDRESP,0,0,0);
      return true;
    }
  }
  time_t now=time(NULL);
  if(now-lastExpireProcess>5)
  {
    lastExpireProcess=now;
    cmd=SmppCommand::makeCommand(PROCESSEXPIREDRESP,0,0,0);
    return true;
  }
  if(!running || queue.Count()==0)return false;
  queue.Pop(cmd);
  return true;
}


}//smpp
}//transport
}//scag
