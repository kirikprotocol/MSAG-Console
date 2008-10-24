#include <scag/util/singleton/Singleton.h>

#include "logger/Logger.h"
#include "util/xml/DOMTreeReader.h"
#include "util/xml/utilFunctions.h"
#include "MmsProcessor.h"
#include "MmsRouter.h"
#include "MmsEntity.h"


namespace scag { namespace transport { namespace mms {

using namespace scag::util::singleton;
using namespace scag::sessions;
using namespace smsc::util::xml;

class MmsProcessorImpl : public MmsProcessor {
public:
  MmsProcessorImpl() {};
  virtual int processRequest(MmsRequest &request);
  virtual int processResponse(MmsResponse &response);
  virtual int statusResponse(MmsResponse &response, bool delivered);
  void init(const string& cfg);
  virtual ~MmsProcessorImpl();

public:
  void addMmsEntity(const MmsEntityInfo& info);
  void updateMmsEntity(const MmsEntityInfo& info);
  void deleteMmsEntity(const char* endpointId) {};
  void ReloadRoutes() {};
  void printMmsEntity();

private:
  MmsEntity** getMmsEntity(const char* endpointId);

private:
  Logger *logger;
  MmsRouter router;
  Hash<MmsEntity*> registry;
  Mutex reg_mutex;
};

static bool inited = false;
static Mutex init_lock;

inline unsigned GetLongevity(MmsProcessor* ) { return 251; };
typedef scag::util::singleton::SingletonHolder<MmsProcessorImpl> SingleMP;


MmsProcessor& MmsProcessor::Instance() {
  {
    MutexGuard g(init_lock);
    if (!inited) {
      throw std::runtime_error("MmsProcessor is not inited!");
    }
  }
  return SingleMP::Instance();
}

void MmsProcessor::Init(const string& cfg) {
  MutexGuard g(init_lock);
  if (!inited) {
    MmsProcessorImpl &mp = SingleMP::Instance();
    mp.init(cfg);
    inited = true;
  }
}

//continue develope form here

int MmsProcessorImpl::processRequest(MmsRequest &request) {
  string endpointId = request.getEndpointId();
  MmsEntity** entity = getMmsEntity(endpointId.c_str());
  if (!entity) {
    smsc_log_warn(logger, "source endpoint=\'%s\' not registered", endpointId.c_str());
    return status::ENDPOINT_NOT_REGISTRED; 
  }
  const MmsRouteInfo* route_info = router.findRouteBySourceEndpointId(endpointId.c_str());
  if (!route_info) {
    smsc_log_warn(logger, "route with source endpointId=\'%s\' not found", endpointId.c_str());
    return status::ROUTE_NOT_FOUND;
  }
  MmsEntity** dest_entity = getMmsEntity(route_info->destId.c_str());
  if (!dest_entity) {
    smsc_log_warn(logger, "destination endpoint=\'%s\' not registered", route_info->destId.c_str());
    return status::ROUTE_NOT_FOUND;
  }
  request.setDestHost((**dest_entity).info.host);
  request.setDestPort((**dest_entity).info.port);
  return scag::re::STATUS_OK;
}

int MmsProcessorImpl::processResponse(MmsResponse &response) {
  //process response
  return scag::re::STATUS_OK;
}

int MmsProcessorImpl::statusResponse(MmsResponse &response, bool delivered) {
  //process statusResponse
  return scag::re::STATUS_OK;
}

enum ParamTag {
  tag_endpointId,
  tag_host,
  tag_port,
  tag_timeout,
  tag_enabled
};

struct ParamsHash: public smsc::core::buffers::Hash<ParamTag>{
  ParamsHash()
  {
    Insert("endpointId", tag_endpointId);
    Insert("host", tag_host);
    Insert("port", tag_port);
    Insert("timeout", tag_timeout);
    Insert("enabled", tag_enabled);
  }
};

static const ParamsHash paramsHash;

void FillStringValue(DOMNamedNodeMap* attr,string& str)
{
  XmlStr value(attr->getNamedItem(XmlStr("value"))->getNodeValue());
  str=value.c_str();
}

int GetIntValue(DOMNamedNodeMap* attr)
{
  XmlStr value(attr->getNamedItem(XmlStr("value"))->getNodeValue());
  return atoi(value.c_str());
}

bool GetBoolValue(DOMNamedNodeMap* attr)
{
  string value;
  FillStringValue(attr, value);
  int len = value.size();
  for(int i = 0; i < len; ++i){
   value[i] = tolower(value[i]);
  }
  if (value.compare("true") == 0) {
    return true;
  }
  if (value.compare("false") == 0) {
    return false;
  }
  throw smsc::util::Exception("Invalid value for param 'enabled':%s", value.c_str());
}

static void ParseTag(MmsProcessorImpl* mmsMan, DOMNodeList* list, MmsEntityType et)
{
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("mmsMan");
  unsigned listLength = list->getLength();
  MmsEntityInfo entity;
  entity.type = et;
  for (unsigned int i=0; i < listLength; ++i) {
    DOMNode *record = list->item(i);
    for(DOMNode *node = record->getFirstChild(); node != 0; node = node->getNextSibling()) {
      if (node->getNodeType() != DOMNode::ELEMENT_NODE) continue;

      DOMNamedNodeMap *attrs = node->getAttributes();
      if(!attrs) {
        throw smsc::util::Exception("Invalid mms.xml configuration file");
      }
      XmlStr paramName(attrs->getNamedItem(XmlStr("name"))->getNodeValue());
      const ParamTag* tagPtr=paramsHash.GetPtr(paramName.c_str());
      if(!tagPtr) {
        throw smsc::util::Exception("Unknown parameter '%s' in mms.xml",paramName.c_str());
      }
      switch(*tagPtr) {
        case tag_endpointId:
          FillStringValue(attrs,entity.endpointId);
          break;
        case tag_host:
          FillStringValue(attrs,entity.host);
          break;
        case tag_port:
          entity.port = GetIntValue(attrs);
          break;
        case tag_timeout:
          entity.timeout = GetIntValue(attrs);
          break;
        case tag_enabled:
          entity.enabled = GetBoolValue(attrs);
          break;
      }
    }
    if (entity.enabled) {
      mmsMan->addMmsEntity(entity);
    } else {
      smsc_log_info(log,"Record with endpointId='%s' disabled and skipped",entity.endpointId.c_str());
    }
  }
}

void MmsProcessorImpl::init(const string& cfg) {
  logger = Logger::getInstance("mms.proc");

  DOMTreeReader reader;
  DOMDocument* doc=reader.read(std::string(cfg + "/mms.xml").c_str());
  DOMElement *elem = doc->getDocumentElement();

  DOMNodeList *list = elem->getElementsByTagName(XmlStr("vasprecord"));
  ParseTag(this,list,etVASP);

  list = elem->getElementsByTagName(XmlStr("rsrecord"));
  ParseTag(this,list,etRS);
  printMmsEntity();

  router.Init(cfg + "/mms_routes.xml");
}

void MmsProcessorImpl::addMmsEntity(const MmsEntityInfo &info) {
  smsc_log_debug(logger, "addMmsEntity:%s", info.endpointId.c_str());
  MutexGuard mg(reg_mutex);
  MmsEntity** ptr = registry.GetPtr(info.endpointId.c_str());
  if (ptr) {
    if ((**ptr).info.type != etUnknown) {
      throw smsc::util::Exception("Duplicate endpointId='%s'", info.endpointId.c_str());
    }
    (**ptr).info = info;
  } else {
    registry.Insert(info.endpointId.c_str(), new MmsEntity(info));
  }
}

void MmsProcessorImpl::updateMmsEntity(const MmsEntityInfo &info) {
  smsc_log_debug(logger, "updateMmsEntity:%s", info.endpointId.c_str());
  MutexGuard mg(reg_mutex);
  MmsEntity** ptr = registry.GetPtr(info.endpointId.c_str());
  if (!ptr) {
    throw smsc::util::Exception("updateMmsEntity:Entity with endpointId='%s' not found",
                                 info.endpointId.c_str());
  }
  MmsEntity &entity = **ptr;
  MutexGuard emg(entity.mutex);
  entity.info = info;
}

MmsEntity** MmsProcessorImpl::getMmsEntity(const char* endpointId) {
  return registry.GetPtr(endpointId);
}

void MmsProcessorImpl::printMmsEntity(){
  char* name = 0;
  MmsEntity* value = 0;
  registry.First();
  while(registry.Next(name, value)) {
    smsc_log_debug(logger, "EndpointId=\'%s\'", name);
    value->info.print(logger);
  }

}

MmsProcessorImpl::~MmsProcessorImpl(){
  char* name = 0;
  MmsEntity* value = 0;
  registry.First();
  while(registry.Next(name, value)) {
    delete value;
  }
}
}//mms
}//transport
}//scag

