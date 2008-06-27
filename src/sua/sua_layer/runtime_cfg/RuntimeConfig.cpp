#include "RuntimeConfig.hpp"
#include <iostream>
#include <stdexcept>
#include <memory>
#include <set>

namespace runtime_cfg {

RuntimeConfig::RuntimeConfig()
  : _config(new CompositeParameter("root")), _wasInitialized(false),
    _logger(smsc::logger::Logger::getInstance("rt_cfg")) {}

void
RuntimeConfig::initialize(CompositeParameter* fullConfiguration)
{
  if ( !_wasInitialized ) {
    _config->addParameter(fullConfiguration);
    _wasInitialized = true;
  }
}

CompositeParameter&
RuntimeConfig::findLastNodeParameter(const std::string& parameterName, std::string* leafParameterName)
{
  std::string::size_type start_token_idx = 0, end_token_idx = parameterName.find('.');
  CompositeParameter* nodeParameter = _config;
  while (end_token_idx != std::string::npos) {
    std::string token = parameterName.substr(start_token_idx, end_token_idx - start_token_idx);

    nodeParameter = nodeParameter->getParameter<CompositeParameter>(token);
    if ( !nodeParameter )
      throw std::runtime_error(std::string("RuntimeConfig::findLastNodeParameter::: node parameter ") + parameterName + std::string(" is not found"));
    start_token_idx = end_token_idx+1; end_token_idx = parameterName.find('.', start_token_idx);
  }
  *leafParameterName = parameterName.substr(start_token_idx, parameterName.size() - start_token_idx);

  return *nodeParameter;
}

void
RuntimeConfig::registerParameterObserver(const std::string& fullParameterName, ParameterObserver* handler)
{
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullParameterName);
  smsc_log_info(_logger, "RuntimeConfig::registerParameterObserver::: register handler for parameter '%s'", fullParameterName.c_str());
  if ( iter == _registredParameterHandlers.end() )
    _registredParameterHandlers.insert(std::make_pair(fullParameterName/*_with_rootPrefix*/, handler));
  else
    iter->second = handler;
}

void
RuntimeConfig::notifyAddParameterEvent(const CompositeParameter& context, Parameter* addedParameter)
{
  std::string fullAddedParameterName = context.getFullName() + "." + addedParameter->getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullAddedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->addParameterEventHandler(context, addedParameter);
  }
}

CompositeParameter*
RuntimeConfig::notifyAddParameterEvent(const CompositeParameter& context, CompositeParameter* addedParameter)
{
  std::string fullAddedParameterName = context.getFullName() + "." + addedParameter->getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullAddedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    return iter->second->addParameterEventHandler(context, addedParameter);
  }
}

void
RuntimeConfig::notifyAddParameterEvent(CompositeParameter* context, Parameter* addedParameter)
{
  std::string fullAddedParameterName = context->getFullName() + "." + addedParameter->getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullAddedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->addParameterEventHandler(context, addedParameter);
  }
}

void
RuntimeConfig::notifyChangeParameterEvent(const CompositeParameter& context, const Parameter& modifiedParameter)
{
  std::string fullModifiedParameterName = context.getFullName() + "." + modifiedParameter.getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullModifiedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->changeParameterEventHandler(context, modifiedParameter);
  }
}

void
RuntimeConfig::notifyChangeParameterEvent(CompositeParameter* context, const Parameter& modifiedParameter)
{
  std::string fullModifiedParameterName = context->getFullName() + "." + modifiedParameter.getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullModifiedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->changeParameterEventHandler(context, modifiedParameter);
  }
}

void
RuntimeConfig::notifyRemoveParameterEvent(const Parameter& removedParameter)
{
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(removedParameter.getFullName());

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->removeParameterEventHandler(removedParameter);
  }
}

void
RuntimeConfig::notifyRemoveParameterEvent(const CompositeParameter& context, const Parameter& removedParameter)
{
  std::string fullRemovedParameterName = context.getFullName() + "." + removedParameter.getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullRemovedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->removeParameterEventHandler(context, removedParameter);
  }
}

void
RuntimeConfig::addParameter(CompositeParameter* compositeParameter, const std::string& paramName, const std::string& paramValue)
{
  std::string::size_type pos, old_pos=0;
  while( (pos=paramValue.find_first_of(", ", old_pos)) != std::string::npos) {
    const std::string& nextValue = paramValue.substr(old_pos, pos - old_pos);

    compositeParameter->addParameter(new Parameter(paramName, nextValue));
    ++pos;
    while (paramValue[pos] == ' ') pos++;
    old_pos = pos;
  }
  const std::string& nextValue = paramValue.substr(old_pos);
  compositeParameter->addParameter(new Parameter(paramName, nextValue));
}

void
RuntimeConfig::processRoutingKeysSection(smsc::util::config::ConfigView* suaLayerCfg,
                                         const char* rkSectionName, // e.g. "routing-keys"
                                         CompositeParameter* suaConfigCompositeParameter
                                         )
{
  std::auto_ptr<smsc::util::config::ConfigView> rksCfg(suaLayerCfg->getSubConfig(rkSectionName));
  CompositeParameter* rkCompositeParameter = suaConfigCompositeParameter->addParameter(new CompositeParameter(rkSectionName));

  std::auto_ptr<std::set<std::string> > topLevelSubSections(rksCfg->getShortSectionNames());

  for (std::set<std::string>::iterator i=topLevelSubSections->begin(), end_iter=topLevelSubSections->end();
       i != end_iter; i++) {
    const char* routeEntry = (*i).c_str();

    CompositeParameter* routingEntryCompositeParameter = rkCompositeParameter->addParameter(new CompositeParameter("routingEntry", routeEntry));
    std::auto_ptr<smsc::util::config::ConfigView> topLevelSubSection(rksCfg->getSubConfig(routeEntry));
    std::auto_ptr<std::set<std::string> > nestedSubSections(topLevelSubSection->getShortSectionNames());
    for (std::set<std::string>::iterator j=nestedSubSections->begin(), nested_end_iter=nestedSubSections->end();
         j != nested_end_iter; j++) {
      const char* routeName = (*j).c_str();
      if ( !strcmp(routeName, "overwrite") ) continue;

      CompositeParameter* routeDescriptionCompositeParameter = routingEntryCompositeParameter->addParameter(new CompositeParameter("route", routeName));
      std::auto_ptr<smsc::util::config::ConfigView> routeDescription(topLevelSubSection->getSubConfig(routeName));

      std::string gt = routeDescription->getString("gt");
      routeDescriptionCompositeParameter->addParameter(new Parameter("gt", gt));

      try {
        int32_t gti = routeDescription->getInt("gti");
        if ( gti > 4 || gti == 0 )
          throw smsc::util::Exception("RuntimeConfig::processRoutingKeysSection::: invalid gti value=[%d]", gti);

        routeDescriptionCompositeParameter->addParameter(new Parameter("gti", gti));
      } catch (smsc::util::config::ConfigException& ex) {}

      try {
        int32_t ssn = routeDescription->getInt("ssn");
        char ssnValue[128];
        sprintf(ssnValue, "%d", ssn);
        routeDescriptionCompositeParameter->addParameter(new Parameter("ssn", ssnValue));
      } catch (smsc::util::config::ConfigException& ex) {}
    }
    CompositeParameter* routeDestinationCompositeParameter = routingEntryCompositeParameter->addParameter(new CompositeParameter("destination"));
    std::string suaApplications;
    try {
      suaApplications = topLevelSubSection->getString("sua_applications");
      addParameter(routingEntryCompositeParameter, "application", suaApplications);
    } catch (smsc::util::config::ConfigException& ex) {}

    std::string sgpLinks;
    try {
      sgpLinks = topLevelSubSection->getString("sgp_links");
      addParameter(routingEntryCompositeParameter, "sgp_link", sgpLinks);
    } catch (smsc::util::config::ConfigException& ex) {}

    std::string trafficMode;
    try {
      trafficMode = topLevelSubSection->getString("traffic-mode");
      routingEntryCompositeParameter->addParameter(new Parameter("traffic-mode", trafficMode));
    } catch (smsc::util::config::ConfigException& ex) {}
  }
}

void
RuntimeConfig::initialize(smsc::util::config::ConfigView* suaLayerCfg)
{
  CompositeParameter* suaConfigCompositeParameter = new CompositeParameter("config");

  suaConfigCompositeParameter->addParameter(new Parameter("local_ip",suaLayerCfg->getString("LocalAddress")));
  suaConfigCompositeParameter->addParameter(new Parameter("local_port",suaLayerCfg->getInt("LocalPort")));

  try {
    suaConfigCompositeParameter->addParameter(new Parameter("lm_ip",suaLayerCfg->getString("LMAddress")));
  } catch (smsc::util::config::ConfigException& ex) {}

  try {
    suaConfigCompositeParameter->addParameter(new Parameter("lm_port",suaLayerCfg->getInt("LMPort")));
  } catch (smsc::util::config::ConfigException& ex) {}

  suaConfigCompositeParameter->addParameter(new Parameter("state_machines_count",suaLayerCfg->getInt("state_machines_count")));
  try {
    suaConfigCompositeParameter->addParameter(new Parameter("traffic-mode-for-sgp",suaLayerCfg->getString("traffic-mode-for-sgp")));
  } catch (smsc::util::config::ConfigException& ex) {}

  std::auto_ptr<smsc::util::config::ConfigView> suaApplicationsCfg(suaLayerCfg->getSubConfig("sua_applications"));

  CompositeParameter* suaAppsCompositeParameter=suaConfigCompositeParameter->addParameter(new CompositeParameter("sua_applications"));

  std::auto_ptr<std::set<std::string> > suaAppsSet(suaApplicationsCfg->getShortSectionNames());

  smsc_log_info(_logger, "RuntimeConfig::initialize::: added sua_applications composite parameter");
  for (std::set<std::string>::iterator i=suaAppsSet->begin(), end_iter=suaAppsSet->end();
       i != end_iter; i++) {
    const std::string& appId = *i;
    suaAppsCompositeParameter->addParameter(new Parameter("application", appId));
    smsc_log_info(_logger, "RuntimeConfig::initialize::: added applications parameter for appId=%s", appId.c_str());
  }

  std::auto_ptr<smsc::util::config::ConfigView> sgpLinksCfg(suaLayerCfg->getSubConfig("sgp_links"));
  CompositeParameter* sgpLinksCompositeParameter = suaConfigCompositeParameter->addParameter(new CompositeParameter("sgp_links"));

  std::auto_ptr<std::set<std::string> > sgpLinksSet(sgpLinksCfg->getShortSectionNames());

  for (std::set<std::string>::iterator i=sgpLinksSet->begin(), end_iter=sgpLinksSet->end();
       i != end_iter; i++) {
    const char* linkId = (*i).c_str();

    CompositeParameter* linkCompositeParameter = sgpLinksCompositeParameter->addParameter(new CompositeParameter("link", linkId));

    smsc_log_info(_logger, "RuntimeConfig::initialize::: added link composite parameter for linkId=%s", linkId);
    std::auto_ptr<smsc::util::config::ConfigView> linkToSGPConfig(sgpLinksCfg->getSubConfig(linkId));

    std::string remoteAddresses(linkToSGPConfig->getString("remote_addresses"));
    std::string::size_type pos, old_pos=0;
    while( (pos=remoteAddresses.find_first_of(", ", old_pos)) != std::string::npos) {
      const std::string& rHost = remoteAddresses.substr(old_pos, pos - old_pos);
      linkCompositeParameter->addParameter(new Parameter("remote_address", rHost));
      ++pos;
      while (remoteAddresses[pos] == ' ') pos++;
      old_pos = pos;
    }
    const std::string& rHost = remoteAddresses.substr(old_pos);
    linkCompositeParameter->addParameter(new Parameter("remote_address", rHost));
    linkCompositeParameter->addParameter(new Parameter("remote_port", linkToSGPConfig->getInt("remote_port")));
    smsc_log_info(_logger, "RuntimeConfig::initialize::: added remote_address parameter [%s]", remoteAddresses.c_str());

    bool localAddressWasConfigured=false;
    try {
      std::string localAddresses(linkToSGPConfig->getString("local_addresses"));
      smsc_log_debug(_logger, "RuntimeConfig::initialize::: processing 'local_addresses' parameter: [%s]", localAddresses.c_str());
      old_pos=0;
      while( (pos=localAddresses.find_first_of(", ", old_pos)) != std::string::npos) {
        const std::string& lHost = localAddresses.substr(old_pos, pos - old_pos);
        smsc_log_debug(_logger, "RuntimeConfig::initialize::: added 'local_address' runtime parameter=[%s]", lHost.c_str());
        linkCompositeParameter->addParameter(new Parameter("local_address", lHost));
        ++pos;
        while (localAddresses[pos] == ' ') pos++;
        old_pos = pos;
      }
      const std::string& lHost = localAddresses.substr(old_pos);
      linkCompositeParameter->addParameter(new Parameter("local_address", lHost));
      localAddressWasConfigured = true;
    } catch (smsc::util::config::ConfigException& ex) {}

    try {
      linkCompositeParameter->addParameter(new Parameter("local_port", linkToSGPConfig->getInt("local_port")));
    } catch (smsc::util::config::ConfigException& ex) {
      if ( localAddressWasConfigured ) throw;
    }
  }

  processRoutingKeysSection(suaLayerCfg, "routing-keys", suaConfigCompositeParameter);

  initialize(suaConfigCompositeParameter);
}

std::string
RuntimeConfig::printConfig() const
{
  return _config->printParamaterValue();
}

}
