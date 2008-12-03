
#include "TemplateManager.h"

namespace smsc {
namespace mcisme {

TemplateManager::TemplateManager(ConfigView* config) // throws ConfigException
{
  // loadup xml config & init formatters

  std::auto_ptr<ConfigView> informCfgGuard(config->getSubConfig("Inform"));
  ConfigView* informCfg = informCfgGuard.get();
  std::auto_ptr< std::set<std::string> > informSetGuard(informCfg->getShortSectionNames());
  std::set<std::string>* informSet = informSetGuard.get();
  //std::set<std::string>* informSet = informCfg->getShortSectionNames();
  //int sz = informSet->size();
  //printf("Total templates is %d\n", sz);

  defaultInformTemplateId = (int32_t)informCfg->getInt("default");
  if (defaultInformTemplateId < 0 || !informTemplates.Exist(defaultInformTemplateId))
    throw ConfigException("Default inform template id=%d is invalid or wasn't specified",
                          defaultInformTemplateId);

  std::string defaultMultiTemplate;
  for (std::set<std::string>::iterator i=informSet->begin();i!=informSet->end();i++)
  {
    const char* templateName = (const char *)i->c_str();
    if (!templateName || !templateName[0])
      throw ConfigException("Inform template name empty or wasn't specified");

    std::auto_ptr<ConfigView> templateConfigGuard(informCfg->getSubConfig(templateName));
    ConfigView* templateConfig = templateConfigGuard.get();

    int  templateId = templateConfig->getInt("id");
    if (templateId < 0 || informTemplates.Exist(templateId))
      throw ConfigException("Inform template id=%d is invalid or already registered", templateId);

    if ( templateId==defaultInformTemplateId ) {
      try {
        defaultMultiTemplate = templateConfig->getString("multiRow"); break;
      } catch (...) {}
    }
  }

  for (std::set<std::string>::iterator i=informSet->begin();i!=informSet->end();i++)
  {
    const char* templateName = (const char *)i->c_str();
    if (!templateName || !templateName[0])
      throw ConfigException("Inform template name empty or wasn't specified");

    std::auto_ptr<ConfigView> templateConfigGuard(informCfg->getSubConfig(templateName));
    ConfigView* templateConfig = templateConfigGuard.get();

    int  templateId = templateConfig->getInt("id");
    if (templateId < 0 || informTemplates.Exist(templateId))
      throw ConfigException("Inform template id=%d is invalid or already registered", templateId);

    bool group      = templateConfig->getBool("group");
    std::string messageTemplate = templateConfig->getString("message");
    std::string unknownCaller   = templateConfig->getString("unknownCaller");
    std::string singleTemplate  = templateConfig->getString("singleRow");

    if (group) {
      std::string multiTemplate = templateConfig->getString("multiRow");
      informTemplates.Insert(templateId,
                             new InformTemplateFormatter(messageTemplate, unknownCaller,
                                                         singleTemplate, multiTemplate));
    } else {
      if (defaultMultiTemplate.empty())
        informTemplates.Insert(templateId,
                               new InformTemplateFormatter(messageTemplate, unknownCaller,
                                                           singleTemplate));
      else
        informTemplates.Insert(templateId,
                               new InformTemplateFormatter(messageTemplate, unknownCaller,
                                                           singleTemplate, defaultMultiTemplate));
    }
    //printf("\ntemplateName = %s\n", templateName);
    //printf("templateId = %d\n", templateId);
    //printf("group = %d\n", group);
    //const char* str = messageTemplate.c_str();
    //printf("messageTemplate = %s (len = %d)\n", messageTemplate.c_str(), messageTemplate.length());
    //const char* tmp = messageTemplate.c_str();
    //printf("unknownCaller = %s\n", unknownCaller.c_str());
    //printf("singleTemplate = %s\n", singleTemplate.c_str());
  }

  std::auto_ptr<ConfigView> notifyCfgGuard(config->getSubConfig("Notify"));
  ConfigView* notifyCfg = notifyCfgGuard.get();
  std::auto_ptr< std::set<std::string> > notifySetGuard(notifyCfg->getShortSectionNames());
  std::set<std::string>* notifySet = notifySetGuard.get();
  for (std::set<std::string>::iterator i=notifySet->begin();i!=notifySet->end();i++)
  {
    const char* templateName = (const char *)i->c_str();
    if (!templateName || !templateName[0])
      throw ConfigException("Notify template name empty or wasn't specified");

    std::auto_ptr<ConfigView> templateConfigGuard(notifyCfg->getSubConfig(templateName));
    ConfigView* templateConfig = templateConfigGuard.get();

    int  templateId = templateConfig->getInt("id");
    if (templateId < 0 || notifyTemplates.Exist(templateId))
      throw ConfigException("Notify template id=%d is invalid or already registered", templateId);

    std::string messageTemplate = templateConfig->getString("message");
    notifyTemplates.Insert(templateId, new NotifyTemplateFormatter(messageTemplate));
  }
  defaultNotifyTemplateId = (int32_t)notifyCfg->getInt("default");
  if (defaultNotifyTemplateId < 0 || !notifyTemplates.Exist(defaultNotifyTemplateId))
    throw ConfigException("Default notify template id=%d is invalid or wasn't specified",
                          defaultNotifyTemplateId);
}

TemplateManager::~TemplateManager() 
{
  // destroys all registereg formatters
  int templateId = 0;
  IntHash<InformTemplateFormatter *>::Iterator informIt = informTemplates.First();
  InformTemplateFormatter* informFormatter = 0;
  while (informIt.Next(templateId, informFormatter))
    if (informFormatter) delete informFormatter;

  IntHash<NotifyTemplateFormatter *>::Iterator notifyIt = notifyTemplates.First();
  NotifyTemplateFormatter* notifyFormatter = 0;
  while (notifyIt.Next(templateId, notifyFormatter))
    if (notifyFormatter) delete notifyFormatter;
}

InformTemplateFormatter* TemplateManager::getInformFormatter(int32_t id)
{
  // if id<0 || formatter not exists => returns default formatter
  InformTemplateFormatter** itf = 0;
  if (id >= 0) {
    itf = informTemplates.GetPtr((int)id);
    if (itf) return *itf;
  }
  itf = informTemplates.GetPtr((int)defaultInformTemplateId);
  return ((itf) ? *itf:0);
}

NotifyTemplateFormatter* TemplateManager::getNotifyFormatter(int32_t id)
{
  // if id<0 || formatter not exists => returns default formatter
  NotifyTemplateFormatter** ntf = 0;
  if (id >= 0) {
    ntf = notifyTemplates.GetPtr((int)id);
    if (ntf) return *ntf;
  }
  ntf = notifyTemplates.GetPtr((int)defaultNotifyTemplateId);
  return ((ntf) ? *ntf:0);
}

}}
