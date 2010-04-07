#include <iostream>
#include <stdexcept>
#include <memory>
#include <set>

#include "RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace runtime_cfg {

RuntimeConfig::RuntimeConfig()
  : _logger(smsc::logger::Logger::getInstance("runtime_cfg"))
{}

void
RuntimeConfig::initialize(smsc::util::config::ConfigView& m3uagw_cfg)
{
  utilx::runtime_cfg::CompositeParameter* m3uaCfgCompositeParam = new utilx::runtime_cfg::CompositeParameter("config");

  m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("local_address", m3uagw_cfg.getString("LocalAddress")));
  m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("local_port", m3uagw_cfg.getInt("LocalPort")));

  try {
    m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("lm_address", m3uagw_cfg.getString("LMAddress")));
  } catch (smsc::util::config::ConfigException& ex) {}

  try {
    m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("lm_port", m3uagw_cfg.getInt("LMPort")));
  } catch (smsc::util::config::ConfigException& ex) {}

  m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("state_machines_count", m3uagw_cfg.getInt("state_machines_count")));
  utilx::runtime_cfg::CompositeParameter* lpcsCompositeParam =
      m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::CompositeParameter("lpcs"));

  parseCommaSeparatedValue(m3uagw_cfg.getString("lpcs"), lpcsCompositeParam, "lpc");

  m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("asp_traffic_mode",m3uagw_cfg.getString("asp_traffic_mode")));

  m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("reassembly_timer", m3uagw_cfg.getInt("reassembly_timer")));

  processSccpUsersSection(m3uagw_cfg, m3uaCfgCompositeParam);

  processSgpLinksSection(m3uagw_cfg, m3uaCfgCompositeParam);

  processPointsDefinitionSection(m3uagw_cfg, m3uaCfgCompositeParam);

  processMtp3RoutingTableSection(m3uagw_cfg, m3uaCfgCompositeParam);

  processTranslationTableSection(m3uagw_cfg, m3uaCfgCompositeParam);

  initialize(m3uaCfgCompositeParam);
}

/**
 * this method results in generation of config records like next:
 * config.sccp_users.userid = SMSC_1
 * config.sccp_users.userid = SMSC_2
 *  ....
 * config.sccp_users.userid = SMSC_N
 */
void
RuntimeConfig::processSccpUsersSection(smsc::util::config::ConfigView& m3ua_cfg,
                                       utilx::runtime_cfg::CompositeParameter* m3ua_cfg_param)
{
  std::auto_ptr<smsc::util::config::ConfigView> sccpUsersCfg(m3ua_cfg.getSubConfig("sccp_users"));

  utilx::runtime_cfg::CompositeParameter* sccpUsersCompositeParam =
      m3ua_cfg_param->addParameter(new utilx::runtime_cfg::CompositeParameter("sccp_users"));

  unsigned count = sccpUsersCfg->getInt("count");
  char paramName[16];
  for(unsigned i=1; i<=count; ++i) {
    sprintf(paramName, "user%d", i);
    char* appId = sccpUsersCfg->getString(paramName);
    sccpUsersCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("userid", appId));
    smsc_log_info(_logger, "RuntimeConfig::initialize::: added applications parameter for appId=%s", appId);
  }
}

/**
 * this method results in generation of config records like next:
 * config.sgp_links.link = LINK_TO_SG1
 * config.sgp_links.link.remote_addresses.address = 192.168.1.1
 * config.sgp_links.link.remote_addresses.address = 192.168.1.2
 * config.sgp_links.link.remote_port = 1234
 * config.sgp_links.link.local_addresses.address = 192.168.1.3
 * config.sgp_links.link.local_addresses.address = 192.168.1.4
 * config.sgp_links.link.local_port = 1234
 *
 * NOTE: local_addresses and local_port composite parameters may be absent
 *
 * config.sgp_links.link = LINK_TO_SG2
 * config records the same as above for link to sgp 2
 */
void
RuntimeConfig::processSgpLinksSection(smsc::util::config::ConfigView& m3ua_cfg,
                                      utilx::runtime_cfg::CompositeParameter* m3ua_cfg_param)
{
  std::auto_ptr<smsc::util::config::ConfigView> sgpLinksCfg(m3ua_cfg.getSubConfig("sgp_links"));
  utilx::runtime_cfg::CompositeParameter* sgpLinksCompositeParameter =
      m3ua_cfg_param->addParameter(new utilx::runtime_cfg::CompositeParameter("sgp_links"));

  std::auto_ptr<std::set<std::string> > sgpLinksSet(sgpLinksCfg->getShortSectionNames());

  for (std::set<std::string>::iterator i=sgpLinksSet->begin(), end_iter=sgpLinksSet->end();
      i != end_iter; i++) {
    const char* linkId = (*i).c_str();

    utilx::runtime_cfg::CompositeParameter* linkCompositeParameter =
        sgpLinksCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter("link", linkId));

    smsc_log_info(_logger, "RuntimeConfig::processSgpLinksSection::: added link composite parameter for linkId=%s", linkId);
    std::auto_ptr<smsc::util::config::ConfigView> linkToSGPConfig(sgpLinksCfg->getSubConfig(linkId));

    parseCommaSeparatedValue(linkToSGPConfig->getString("remote_addresses"),
                             linkCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter("remote_addresses")),
                             "address");
    linkCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("remote_port", linkToSGPConfig->getInt("remote_port")));

    bool localAddressWasConfigured=false;
    try {
      parseCommaSeparatedValue(linkToSGPConfig->getString("local_addresses"),
                               linkCompositeParameter->addParameter(new utilx::runtime_cfg::CompositeParameter("local_addresses")),
                               "address");
      localAddressWasConfigured = true;
      linkCompositeParameter->addParameter(new utilx::runtime_cfg::Parameter("local_port", linkToSGPConfig->getInt("local_port")));
    } catch (smsc::util::config::ConfigException& ex) {
      if ( localAddressWasConfigured ) throw;
    }
  }
}

/**
 * this method results in generation of config records like next:
 * config.mtp3-routing-tables.table = LPC_133
 * config.mtp3-routing-tables.table.lpc = 133

 * config.mtp3-routing-tables.table.entry = route_entry_1
 * config.mtp3-routing-tables.table.entry.dpc = 132
 * config.mtp3-routing-tables.table.entry.sgp_link = LINK_TO_SG1

 * config.mtp3-routing-tables.table.entry = route_entry_2
 * config.mtp3-routing-tables.table.entry.dpc = 131
 * config.mtp3-routing-tables.table.entry.sgp_link = LINK_TO_SG2
 */
void
RuntimeConfig::processMtp3RoutingTableSection(smsc::util::config::ConfigView& m3ua_cfg,
                                              utilx::runtime_cfg::CompositeParameter* m3ua_cfg_param)
{
  std::auto_ptr<smsc::util::config::ConfigView> mtp3RoutingTablesCfg(m3ua_cfg.getSubConfig("mtp3-routing-tables"));
  utilx::runtime_cfg::CompositeParameter* mtp3RoutingTablesCompositeParam =
      m3ua_cfg_param->addParameter(new utilx::runtime_cfg::CompositeParameter("mtp3-routing-tables"));

  std::auto_ptr<std::set<std::string> > routingTablesSet(mtp3RoutingTablesCfg->getShortSectionNames());

  for (std::set<std::string>::iterator i=routingTablesSet->begin(), end_iter=routingTablesSet->end();
      i != end_iter; i++) {
    const char* routingTableName = (*i).c_str();
    utilx::runtime_cfg::CompositeParameter* routeTableCompositParam =
        mtp3RoutingTablesCompositeParam->addParameter(new utilx::runtime_cfg::CompositeParameter("table",
                                                                                                 routingTableName));
    std::auto_ptr<smsc::util::config::ConfigView> routeTableConfig(mtp3RoutingTablesCfg->getSubConfig(routingTableName)); // correspond to <section name="LPC_133"> for example
    routeTableCompositParam->addParameter(new utilx::runtime_cfg::Parameter("lpc", routeTableConfig->getInt("lpc")));
    std::auto_ptr<std::set<std::string> > routeEntriesSet(routeTableConfig->getShortSectionNames());
    for (std::set<std::string>::iterator j=routeEntriesSet->begin(), end_iter2=routeEntriesSet->end();
          j != end_iter2; j++) {
      const char* routeEntryName = (*j).c_str();
      utilx::runtime_cfg::CompositeParameter* routeEntryCompositParam =
          routeTableCompositParam->addParameter(new utilx::runtime_cfg::CompositeParameter("entry", routeEntryName));
      std::auto_ptr<smsc::util::config::ConfigView> routeEntryConfig(routeTableConfig->getSubConfig(routeEntryName));  // correspond to <section name="route_entry_1"> for example
      routeEntryCompositParam->addParameter(new utilx::runtime_cfg::Parameter("dpc", routeEntryConfig->getInt("dpc")));
      routeEntryCompositParam->addParameter(new utilx::runtime_cfg::Parameter("sgp_link", routeEntryConfig->getString("sgp_link")));
    }
  }
}

/**
 * this method results in generation of config records like next:
 * config.translation-table.entry = MasterGT
 * config.translation-table.entry.gt = +79139869990
 * config.translation-table.entry.sccp_users.userid = SMSC_PRIMARY
 * config.translation-table.entry.sccp_users.userid = SMSC_BACKUP
 * config.translation-table.entry.traffic-mode = loadshare
 *
 * config.translation-table.entry = SMSC1
 * config.translation-table.entry.gt = +79139869991
 * config.translation-table.entry.ssn = 8
 * config.translation-table.entry.sccp_users.userid = SMSC_PRIMARY
 * config.translation-table.entry.traffic-mode = dedicated
 *
 * config.translation-table.entry = CLOUD
 * config.translation-table.entry.gt = +79139869991
 * config.translation-table.entry.lpc = 133
 * config.translation-table.entry.dpcs.pc = 131
 * config.translation-table.entry.dpcs.pc = 132
 * config.translation-table.entry.traffic-mode = loadshare
 */
void
RuntimeConfig::processTranslationTableSection(smsc::util::config::ConfigView& m3ua_cfg,
                                              utilx::runtime_cfg::CompositeParameter* m3ua_cfg_param)
{
  std::auto_ptr<smsc::util::config::ConfigView> translationTableCfg(m3ua_cfg.getSubConfig("translation-table"));
  utilx::runtime_cfg::CompositeParameter* translationTableCompositeParam =
      m3ua_cfg_param->addParameter(new utilx::runtime_cfg::CompositeParameter("translation-table"));

  std::auto_ptr<std::set<std::string> > topLevelSubSections(translationTableCfg->getShortSectionNames());

  for (std::set<std::string>::iterator i=topLevelSubSections->begin(), end_iter=topLevelSubSections->end();
       i != end_iter; i++) {
    const char* routeEntryName = (*i).c_str();

    utilx::runtime_cfg::CompositeParameter* routeEntryCompositeParam =
        translationTableCompositeParam->addParameter(new utilx::runtime_cfg::CompositeParameter("entry", routeEntryName));
    std::auto_ptr<smsc::util::config::ConfigView> routeEntrySubSection(translationTableCfg->getSubConfig(routeEntryName));
    routeEntryCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("gt", routeEntrySubSection->getString("gt")));
    routeEntryCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("traffic-mode", routeEntrySubSection->getString("traffic-mode")));
    routeEntrySubSection->getString("traffic-mode");
    try {
      // if sccp_users present then it's route to application else it's route to ss7 network
      // and lpc/dpc must be present
      const char* sccpUsersValue = routeEntrySubSection->getString("sccp_users");
      utilx::runtime_cfg::CompositeParameter* sccpUsersCompositeParam =
          routeEntryCompositeParam->addParameter(new utilx::runtime_cfg::CompositeParameter("sccp_users"));
      parseCommaSeparatedValue(sccpUsersValue, sccpUsersCompositeParam, "userid");
      try {
        routeEntryCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("ssn",
                                                                                 routeEntrySubSection->getInt("ssn")));
      } catch (smsc::util::config::ConfigException& ex) {}

    } catch (smsc::util::config::ConfigException& ex) {
      const char* dpcsValue = routeEntrySubSection->getString("dpcs");
      utilx::runtime_cfg::CompositeParameter* dpcsCompositeParam =
          routeEntryCompositeParam->addParameter(new utilx::runtime_cfg::CompositeParameter("dpcs"));
      parseCommaSeparatedValue(dpcsValue, dpcsCompositeParam, "pc");
      routeEntryCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("lpc",
                                                                               routeEntrySubSection->getInt("lpc")));
    }
  }
}

/**
 * this method results in generation of config records like next:
 * config.points-definition.point = LPC_11044
 * config.points-definition.point.lpc = 11044
 * config.points-definition.point.ni = 3
 * config.points-definition.point.standard = ITU
 */
void
RuntimeConfig::processPointsDefinitionSection(smsc::util::config::ConfigView& m3ua_cfg,
                                              utilx::runtime_cfg::CompositeParameter* m3ua_cfg_param)
{
  std::auto_ptr<smsc::util::config::ConfigView> pointsDefinitionCfg(m3ua_cfg.getSubConfig("points-definition"));
  utilx::runtime_cfg::CompositeParameter* pointsDefinitionCompositeParam =
      m3ua_cfg_param->addParameter(new utilx::runtime_cfg::CompositeParameter("points-definition"));

  std::auto_ptr<std::set<std::string> > topLevelSubSections(pointsDefinitionCfg->getShortSectionNames());

  for (std::set<std::string>::iterator i=topLevelSubSections->begin(), end_iter=topLevelSubSections->end();
       i != end_iter; i++) {
    const char* pointName = (*i).c_str();
    utilx::runtime_cfg::CompositeParameter* pointCompositeParam =
        pointsDefinitionCompositeParam->addParameter(new utilx::runtime_cfg::CompositeParameter("point", pointName));
    std::auto_ptr<smsc::util::config::ConfigView> pointSubSection(pointsDefinitionCfg->getSubConfig(pointName));
    pointCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("lpc", pointSubSection->getInt("lpc")));
    pointCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("ni", pointSubSection->getInt("ni")));
    pointCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("standard", pointSubSection->getString("standard")));
  }
}

}}}}
