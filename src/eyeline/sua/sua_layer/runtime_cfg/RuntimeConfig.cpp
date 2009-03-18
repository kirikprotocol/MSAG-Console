#include "RuntimeConfig.hpp"
#include <iostream>
#include <stdexcept>
#include <memory>
#include <set>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace runtime_cfg {

RuntimeConfig::RuntimeConfig()
  : _logger(smsc::logger::Logger::getInstance("runtime_cfg"))
{}

void
RuntimeConfig::processRoutingKeysSection(smsc::util::config::ConfigView* suaLayerCfg,
                                         const char* rkSectionName, // e.g. "routing-keys"
                                         utilx::runtime_cfg::CompositeParameter* suaConfigCompositeParameter
                                         )
{
  std::auto_ptr<smsc::util::config::ConfigView> rksCfg(suaLayerCfg->getSubConfig(rkSectionName));
  utilx::runtime_cfg::CompositeParameter* rkCompositeParameter =
    suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter(rkSectionName));

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
RuntimeConfig::initialize(smsc::util::config::ConfigView* suaLayerCfg)
{
  utilx::runtime_cfg::CompositeParameter* suaConfigCompositeParameter = new utilx::runtime_cfg::CompositeParameter("config");

  suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("local_ip",suaLayerCfg->getString("LocalAddress")));
  suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("local_port",suaLayerCfg->getInt("LocalPort")));

  try {
    suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("lm_ip",suaLayerCfg->getString("LMAddress")));
  } catch (smsc::util::config::ConfigException& ex) {}

  try {
    suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("lm_port",suaLayerCfg->getInt("LMPort")));
  } catch (smsc::util::config::ConfigException& ex) {}

  suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("state_machines_count",suaLayerCfg->getInt("state_machines_count")));
  try {
    suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("traffic-mode-for-sgp",suaLayerCfg->getString("traffic-mode-for-sgp")));
  } catch (smsc::util::config::ConfigException& ex) {}

  std::auto_ptr<smsc::util::config::ConfigView> suaApplicationsCfg(suaLayerCfg->getSubConfig("sua_applications"));

  utilx::runtime_cfg::CompositeParameter* suaAppsCompositeParameter =
    suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter("sua_applications"));

  std::auto_ptr<std::set<std::string> > suaAppsSet(suaApplicationsCfg->getShortSectionNames());

  smsc_log_info(_logger, "RuntimeConfig::initialize::: added sua_applications composite parameter");
  for (std::set<std::string>::iterator i=suaAppsSet->begin(), end_iter=suaAppsSet->end();
       i != end_iter; i++) {
    const std::string& appId = *i;
    suaAppsCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("application", appId));
    smsc_log_info(_logger, "RuntimeConfig::initialize::: added applications parameter for appId=%s", appId.c_str());
  }

  std::auto_ptr<smsc::util::config::ConfigView> sgpLinksCfg(suaLayerCfg->getSubConfig("sgp_links"));
  utilx::runtime_cfg::CompositeParameter* sgpLinksCompositeParameter =
    suaConfigCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter("sgp_links"));

  std::auto_ptr<std::set<std::string> > sgpLinksSet(sgpLinksCfg->getShortSectionNames());

  for (std::set<std::string>::iterator i=sgpLinksSet->begin(), end_iter=sgpLinksSet->end();
       i != end_iter; i++) {
    const char* linkId = (*i).c_str();

    utilx::runtime_cfg::CompositeParameter* linkCompositeParameter =
      sgpLinksCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter("link", linkId));

    smsc_log_info(_logger, "RuntimeConfig::initialize::: added link composite parameter for linkId=%s", linkId);
    std::auto_ptr<smsc::util::config::ConfigView> linkToSGPConfig(sgpLinksCfg->getSubConfig(linkId));

    std::string remoteAddresses(linkToSGPConfig->getString("remote_addresses"));
    std::string::size_type pos, old_pos=0;
    while( (pos=remoteAddresses.find_first_of(", ", old_pos)) != std::string::npos) {
      const std::string& rHost = remoteAddresses.substr(old_pos, pos - old_pos);
      linkCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("remote_address", rHost));
      ++pos;
      while (remoteAddresses[pos] == ' ') pos++;
      old_pos = pos;
    }
    const std::string& rHost = remoteAddresses.substr(old_pos);
    linkCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("remote_address", rHost));
    linkCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("remote_port", linkToSGPConfig->getInt("remote_port")));
    smsc_log_info(_logger, "RuntimeConfig::initialize::: added remote_address parameter [%s]", remoteAddresses.c_str());

    bool localAddressWasConfigured=false;
    try {
      std::string localAddresses(linkToSGPConfig->getString("local_addresses"));
      smsc_log_debug(_logger, "RuntimeConfig::initialize::: processing 'local_addresses' parameter: [%s]", localAddresses.c_str());
      old_pos=0;
      while( (pos=localAddresses.find_first_of(", ", old_pos)) != std::string::npos) {
        const std::string& lHost = localAddresses.substr(old_pos, pos - old_pos);
        smsc_log_debug(_logger, "RuntimeConfig::initialize::: added 'local_address' runtime parameter=[%s]", lHost.c_str());
        linkCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("local_address", lHost));
        ++pos;
        while (localAddresses[pos] == ' ') pos++;
        old_pos = pos;
      }
      const std::string& lHost = localAddresses.substr(old_pos);
      linkCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("local_address", lHost));
      localAddressWasConfigured = true;
    } catch (smsc::util::config::ConfigException& ex) {}

    try {
      linkCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("local_port", linkToSGPConfig->getInt("local_port")));
    } catch (smsc::util::config::ConfigException& ex) {
      if ( localAddressWasConfigured ) throw;
    }
  }

  processRoutingKeysSection(suaLayerCfg, "routing-keys", suaConfigCompositeParameter);

  initialize(suaConfigCompositeParameter);
}

}}}}
