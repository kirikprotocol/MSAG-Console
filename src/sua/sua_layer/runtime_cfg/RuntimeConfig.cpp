#include "RuntimeConfig.hpp"
#include <iostream>
#include <stdexcept>
#include <memory>
#include <set>

namespace runtime_cfg {

RuntimeConfig*
utilx::Singleton<RuntimeConfig>::_instance;

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

    //    std::cout << "RuntimeConfig::findLastNodeParameter::: call nodeParameter.getParameter(" << token << ")" << std::endl;
    nodeParameter = nodeParameter->getParameter<CompositeParameter>(token);
    if ( !nodeParameter )
      throw std::runtime_error(std::string("RuntimeConfig::findLastNodeParameter::: node parameter ") + parameterName + std::string(" is not found"));
    start_token_idx = end_token_idx+1; end_token_idx = parameterName.find('.', start_token_idx);
  }
  *leafParameterName = parameterName.substr(start_token_idx, parameterName.size() - start_token_idx);

  return *nodeParameter;
}

template <> CompositeParameter&
RuntimeConfig::find<CompositeParameter>(const std::string& parameterName) {
  std::string leafParameterName;
  CompositeParameter& nodeParameter = findLastNodeParameter(parameterName, &leafParameterName);

  CompositeParameter* nodeParameterPtr = nodeParameter.getParameter<CompositeParameter>(leafParameterName);
  if (  !nodeParameterPtr )
    throw std::runtime_error(std::string("C::find<CompositeParameter>::: node parameter ") + parameterName + std::string(" is not found"));

  return *nodeParameterPtr;

}

template <> Parameter&
RuntimeConfig::find<Parameter>(const std::string& parameterName) {
  std::string leafParameterName;
  CompositeParameter& nodeParameter = findLastNodeParameter(parameterName, &leafParameterName);

  Parameter* childParameterPtr = nodeParameter.getParameter<Parameter>(leafParameterName);
  if (  !childParameterPtr )
    throw std::runtime_error(std::string("C::find<Parameter>::: leaf parameter ") + parameterName + std::string(" is not found"));

  return *childParameterPtr;
}

void
RuntimeConfig::registerParameterObserver(const std::string& fullParameterName, ParameterObserver* handler)
{
  std::string fullParameterName_with_rootPrefix = std::string("root.") + fullParameterName;
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullParameterName_with_rootPrefix);
  //  std::cout << "RuntimeConfig::registerParameterObserver::: register handler for parameter name = " << fullParameterName_with_rootPrefix << std::endl;
  if ( iter == _registredParameterHandlers.end() )
    _registredParameterHandlers.insert(std::make_pair(fullParameterName_with_rootPrefix, handler));
  else
    iter->second = handler;
}

void
RuntimeConfig::dispatchHandle(const Parameter& modifiedParameter)
{
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(modifiedParameter.getFullName());
  //  std::cout << "RuntimeConfig::registerParameterObserver::: get handler for parameter name = " << modifiedParameter.getFullName() << std::endl;
  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->handle(modifiedParameter);
  }
}

void
RuntimeConfig::processRoutingKeysSection(smsc::util::config::ConfigView* suaLayerCfg,
                                         const char* rkSectionName, // e.g. "incoming-routing-keys"
                                         const char* linksParamNameInRKSection,// e.g. "sua_applications"
                                         const char* linkRTCfgParamName, // e.g. "application_id"
                                         CompositeParameter* suaConfigCompositeParameter
                                         )
{
  std::auto_ptr<smsc::util::config::ConfigView> rksCfg(suaLayerCfg->getSubConfig(rkSectionName));
  CompositeParameter* rkCompositeParameter = suaConfigCompositeParameter->addParameter(new CompositeParameter(rkSectionName));

  std::auto_ptr<std::set<std::string> > gtSet(rksCfg->getShortSectionNames());

  for (std::set<std::string>::iterator i=gtSet->begin(), end_iter=gtSet->end();
       i != end_iter; i++) {
    const char* gt = (*i).c_str();
    std::auto_ptr<smsc::util::config::ConfigView> gtConfig(rksCfg->getSubConfig(gt));
    CompositeParameter* gtCompositeParameter = rkCompositeParameter->addParameter(new CompositeParameter("GT", gt));

    std::string rk_linksParam = gtConfig->getString(linksParamNameInRKSection);
    std::string::size_type pos, old_pos=0;
    while( (pos=rk_linksParam.find_first_of(", ", old_pos)) != std::string::npos) {
      const std::string& paramName = rk_linksParam.substr(old_pos, pos - old_pos);
      gtCompositeParameter->addParameter(new Parameter(linkRTCfgParamName, paramName));
      ++pos;
      while (rk_linksParam[pos] == ' ') pos++;
      old_pos = pos;
    }
    const std::string& paramName = rk_linksParam.substr(old_pos);
    gtCompositeParameter->addParameter(new Parameter(linkRTCfgParamName, paramName));
    //    smsc_log_info(_logger, "added Parameter('%s', '%s')", linkRTCfgParamName, paramName.c_str());

    std::string rk_traffic_mode = gtConfig->getString("traffic-mode");
    gtCompositeParameter->addParameter(new Parameter("traffic_mode", rk_traffic_mode));
    //    smsc_log_info(_logger, "added Parameter('traffic_mode', '%s')", rk_traffic_mode.c_str());
  }
}
/*
** config.local_ip
** config.local_port
** config.state_machines_count
** [config.sua_applications.application]1..*
** [config.sgp_links.remote_address]1..*
** config.sgp_links.remote_port
** [incoming-routing-keys.GT.application_id]1..*
** incoming-routing-keys.GT.traffic_mode
** [outcoming-routing-keys.GT.link_id]1..*
** outcoming-routing-keys.GT.traffic_mode
*/
void
RuntimeConfig::initialize(smsc::util::config::ConfigView* suaLayerCfg)
{
  CompositeParameter* suaConfigCompositeParameter = new CompositeParameter("config");

  suaConfigCompositeParameter->addParameter(new Parameter("local_ip",suaLayerCfg->getString("LocalAddress")));
  suaConfigCompositeParameter->addParameter(new Parameter("local_port",suaLayerCfg->getInt("LocalPort")));
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

  processRoutingKeysSection(suaLayerCfg, "incoming-routing-keys", "sua_applications", "application_id", suaConfigCompositeParameter);
  processRoutingKeysSection(suaLayerCfg, "outcoming-routing-keys", "sgp_links", "link_id", suaConfigCompositeParameter);

  initialize(suaConfigCompositeParameter);
}

}
