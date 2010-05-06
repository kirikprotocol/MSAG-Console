#include <iostream>
#include <stdexcept>
#include <memory>
#include <set>

#include "RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace runtime_cfg {

RuntimeConfig::RuntimeConfig()
  : _logger(smsc::logger::Logger::getInstance("runtime_cfg"))
{}

void
RuntimeConfig::processRoutingKeysSection(smsc::util::config::ConfigView& sua_layer_cfg,
                                         const char* rk_section_name, // e.g. "routing-keys"
                                         utilx::runtime_cfg::CompositeParameter* sua_cfg_composite_param
                                         )
{
  std::auto_ptr<smsc::util::config::ConfigView> rksCfg(sua_layer_cfg.getSubConfig(rk_section_name));
  utilx::runtime_cfg::CompositeParameter* rkCompositeParameter =
    sua_cfg_composite_param->addParameter(new utilx::runtime_cfg::CompositeParameter(rk_section_name));

  std::auto_ptr<std::set<std::string> > topLevelSubSections(rksCfg->getShortSectionNames());

  for (std::set<std::string>::iterator i=topLevelSubSections->begin(), end_iter=topLevelSubSections->end();
       i != end_iter; i++) {
    const char* routeEntry = (*i).c_str();

    utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter =
      rkCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter("routingEntry", routeEntry));
    std::auto_ptr<smsc::util::config::ConfigView> topLevelSubSection(rksCfg->getSubConfig(routeEntry));
    std::auto_ptr<std::set<std::string> > nestedSubSections(topLevelSubSection->getShortSectionNames());
    for (std::set<std::string>::iterator j=nestedSubSections->begin(), nested_end_iter=nestedSubSections->end();
         j != nested_end_iter; j++) {
      const char* routeName = (*j).c_str();
      if ( !strcmp(routeName, "overwrite") ) continue;

      utilx::runtime_cfg::CompositeParameter* routeDescriptionCompositeParameter =
        routingEntryCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter("route", routeName));
      std::auto_ptr<smsc::util::config::ConfigView> routeDescription(topLevelSubSection->getSubConfig(routeName));

      std::string gt = routeDescription->getString("gt");
      routeDescriptionCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("gt", gt));

      try {
        int32_t gti = routeDescription->getInt("gti");
        if ( gti > 4 || gti == 0 )
          throw smsc::util::Exception("RuntimeConfig::processRoutingKeysSection::: invalid gti value=[%d]", gti);

        routeDescriptionCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("gti", gti));
      } catch (smsc::util::config::ConfigException& ex) {}

      try {
        int32_t ssn = routeDescription->getInt("ssn");
        char ssnValue[128];
        sprintf(ssnValue, "%d", ssn);
        routeDescriptionCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("ssn", ssnValue));
      } catch (smsc::util::config::ConfigException& ex) {}
    }
    utilx::runtime_cfg::CompositeParameter* routeDestinationCompositeParameter =
      routingEntryCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter("destination"));
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
      routingEntryCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("traffic-mode", trafficMode));
    } catch (smsc::util::config::ConfigException& ex) {}
  }
}

void
RuntimeConfig::initialize(smsc::util::config::ConfigView& sua_layer_cfg,
                          const char* cfg_file)
{
  utilx::runtime_cfg::CompositeParameter* suaConfigCompositeParameter = new utilx::runtime_cfg::CompositeParameter("config");

  suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("local_ip",sua_layer_cfg.getString("LocalAddress")));
  suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("local_port",sua_layer_cfg.getInt("LocalPort")));

  try {
    suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("lm_ip",sua_layer_cfg.getString("LMAddress")));
  } catch (smsc::util::config::ConfigException& ex) {}

  try {
    suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("lm_port",sua_layer_cfg.getInt("LMPort")));
  } catch (smsc::util::config::ConfigException& ex) {}

  suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("state_machines_count",sua_layer_cfg.getInt("state_machines_count")));
  try {
    suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("traffic-mode-for-sgp",sua_layer_cfg.getString("traffic-mode-for-sgp")));
  } catch (smsc::util::config::ConfigException& ex) {}

  processSuaApplications(sua_layer_cfg, suaConfigCompositeParameter);

  processSgpLinks(sua_layer_cfg, suaConfigCompositeParameter);

  processRoutingKeysSection(sua_layer_cfg, "routing-keys", suaConfigCompositeParameter);

  initialize(suaConfigCompositeParameter);
}

void
RuntimeConfig::processSuaApplications(smsc::util::config::ConfigView& sua_layer_cfg,
                                      utilx::runtime_cfg::CompositeParameter* sua_cfg_composite_param)
{
  std::auto_ptr<smsc::util::config::ConfigView> suaApplicationsCfg(sua_layer_cfg.getSubConfig("sua_applications"));

  utilx::runtime_cfg::CompositeParameter* suaAppsCompositeParameter =
      sua_cfg_composite_param->addParameter(new utilx::runtime_cfg::CompositeParameter("sua_applications"));

  std::auto_ptr<std::set<std::string> > suaAppsSet(suaApplicationsCfg->getShortSectionNames());

  smsc_log_info(_logger, "RuntimeConfig::processSuaApplications::: added sua_applications composite parameter");
  for (std::set<std::string>::iterator i=suaAppsSet->begin(), end_iter=suaAppsSet->end();
       i != end_iter; i++) {
    const std::string& appId = *i;
    suaAppsCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("application", appId));
    smsc_log_info(_logger, "RuntimeConfig::processSuaApplications::: added applications parameter for appId=%s", appId.c_str());
  }
}

void
RuntimeConfig::processSgpLinks(smsc::util::config::ConfigView& sua_layer_cfg,
                               utilx::runtime_cfg::CompositeParameter* sua_cfg_composite_param)
{
  std::auto_ptr<smsc::util::config::ConfigView> sgpLinksCfg(sua_layer_cfg.getSubConfig("sgp_links"));
  utilx::runtime_cfg::CompositeParameter* sgpLinksCompositeParameter =
    sua_cfg_composite_param->addParameter(new utilx::runtime_cfg::CompositeParameter("sgp_links"));

  std::auto_ptr<std::set<std::string> > sgpLinksSet(sgpLinksCfg->getShortSectionNames());

  for (std::set<std::string>::iterator i=sgpLinksSet->begin(), end_iter=sgpLinksSet->end();
       i != end_iter; i++) {
    const char* linkId = (*i).c_str();

    utilx::runtime_cfg::CompositeParameter* linkCompositeParameter =
      sgpLinksCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter("link", linkId));

    smsc_log_info(_logger, "RuntimeConfig::processSgpLinks::: added link composite parameter for linkId=%s", linkId);
    std::auto_ptr<smsc::util::config::ConfigView> linkToSGPConfig(sgpLinksCfg->getSubConfig(linkId));

    std::string remoteAddresses(linkToSGPConfig->getString("remote_addresses"));
    parseCommaSeparatedValue(remoteAddresses,
                             linkCompositeParameter,
                             "remote_address");
    linkCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("remote_port", linkToSGPConfig->getInt("remote_port")));
    smsc_log_info(_logger, "RuntimeConfig::processSgpLinks::: added remote_address parameter [%s]", remoteAddresses.c_str());

    bool localAddressWasConfigured=false;
    try {
      std::string localAddresses(linkToSGPConfig->getString("local_addresses"));
      smsc_log_debug(_logger, "RuntimeConfig::processSgpLinks::: processing 'local_addresses' parameter: [%s]", localAddresses.c_str());
      parseCommaSeparatedValue(localAddresses,
                               linkCompositeParameter,
                               "local_address");
      localAddressWasConfigured = true;
    } catch (smsc::util::config::ConfigException& ex) {}

    try {
      linkCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("local_port", linkToSGPConfig->getInt("local_port")));
    } catch (smsc::util::config::ConfigException& ex) {
      if ( localAddressWasConfigured ) throw;
    }
  }
}

}}}}
