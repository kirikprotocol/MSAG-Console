#include <iostream>
#include <stdexcept>
#include <memory>
#include <set>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "RuntimeConfig.hpp"
#include "util/config/Config.h"
#include "util/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace runtime_cfg {

RuntimeConfig::RuntimeConfig()
  : _logger(smsc::logger::Logger::getInstance("runtime_cfg")), _cfgFile(NULL)
{}

void
RuntimeConfig::initialize(smsc::util::config::ConfigView& m3uagw_cfg,
                          const char* cfg_file)
{
  _cfgFile = cfg_file;
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

  m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("asp_traffic_mode",m3uagw_cfg.getString("asp_traffic_mode")));

  m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("reassembly_timer", m3uagw_cfg.getInt("reassembly_timer")));

  try {
    m3uaCfgCompositeParam->addParameter(new utilx::runtime_cfg::Parameter("reconnect_interval",
                                                                          m3uagw_cfg.getInt("reconnect_interval")));
  } catch (smsc::util::config::ConfigException& ex) {}

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
      parseCommaSeparatedIntValue(dpcsValue, dpcsCompositeParam, "pc");
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

void
RuntimeConfig::commit()
{
  smsc::util::config::Config xmlConf;

  xmlConf.setString("M3uaGW.LocalAddress",
                    find<utilx::runtime_cfg::Parameter>("config.local_address").getValue().c_str());
  xmlConf.setInt("M3uaGW.LocalPort",
                 find<utilx::runtime_cfg::Parameter>("config.local_port").getIntValue());

  try {
    xmlConf.setString("M3uaGW.LMAddress",
                      find<utilx::runtime_cfg::Parameter>("config.lm_address").getValue().c_str());
  } catch (std::runtime_error& ex) {}

  try {
    xmlConf.setInt("M3uaGW.LMPort",
                   find<utilx::runtime_cfg::Parameter>("config.lm_port").getIntValue());
  } catch (std::runtime_error& ex) {}

  xmlConf.setInt("M3uaGW.state_machines_count",
                 find<utilx::runtime_cfg::Parameter>("config.state_machines_count").getIntValue());

  xmlConf.setString("M3uaGW.asp_traffic_mode",
                    find<utilx::runtime_cfg::Parameter>("config.asp_traffic_mode").getValue().c_str());

  xmlConf.setInt("M3uaGW.reassembly_timer",
                 find<utilx::runtime_cfg::Parameter>("config.reassembly_timer").getIntValue());

  xmlConf.setInt("M3uaGW.reconnect_interval",
                 find<utilx::runtime_cfg::Parameter>("config.reconnect_interval").getIntValue());

  prepareSccpUsersSection(&xmlConf);

  prepareSgpLinksSection(&xmlConf);

  preparePointsDefinition(&xmlConf);

  prepareMtp3RoutingTable(&xmlConf);

  prepareTranslationTable(&xmlConf);

  saveXmlConfig(xmlConf);
}

void
RuntimeConfig::prepareSccpUsersSection(smsc::util::config::Config* xml_conf)
{
  utilx::runtime_cfg::CompositeParameter& sccpUsers =
      find<utilx::runtime_cfg::CompositeParameter>("config.sccp_users");

  unsigned totalUserIds = 0;
  {
    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter>
      userIdIter = sccpUsers.getIterator<utilx::runtime_cfg::Parameter>("userid");
    while ( userIdIter.hasElement() ) {
      ++totalUserIds; userIdIter.next();
    }
  }

  xml_conf->setInt("M3uaGW.sccp_users.count", totalUserIds);
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter>
    userIdIter = sccpUsers.getIterator<utilx::runtime_cfg::Parameter>("userid");

  for ( int userIdx=1; userIdIter.hasElement(); userIdx++, userIdIter.next() ) {
    char xmlParamName[64];
    sprintf(xmlParamName, "M3uaGW.sccp_users.user%d", userIdx);
    xml_conf->setString(xmlParamName, userIdIter.getCurrentElement()->getValue().c_str());
  }
}

void
RuntimeConfig::prepareSgpLinksSection(smsc::util::config::Config* xml_conf)
{
  /*
   * config.sgp_links.link = LINK_TO_SG1
   * config.sgp_links.link.remote_addresses.address = 192.168.1.1
   * config.sgp_links.link.remote_addresses.address = 192.168.1.2
   * config.sgp_links.link.remote_port = 1234
   * config.sgp_links.link.local_addresses.address = 192.168.1.3
   * config.sgp_links.link.local_addresses.address = 192.168.1.4
   * config.sgp_links.link.local_port = 1234
   */
  utilx::runtime_cfg::CompositeParameter& sgpLinks =
      find<utilx::runtime_cfg::CompositeParameter>("config.sgp_links");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
  linkIter = sgpLinks.getIterator<utilx::runtime_cfg::CompositeParameter>("link");

  while ( linkIter.hasElement() ) {
    utilx::runtime_cfg::CompositeParameter* linkParam = linkIter.getCurrentElement();
    utilx::runtime_cfg::CompositeParameter* remoteAddrs =
        linkParam->getParameter<utilx::runtime_cfg::CompositeParameter>("remote_addresses");
    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter>
    remoteAddrIter = remoteAddrs->getIterator<utilx::runtime_cfg::Parameter>("address");
    std::string remoteAddrVals;
    while ( remoteAddrIter.hasElement() ) {
      remoteAddrVals += remoteAddrIter.getCurrentElement()->getValue();
      remoteAddrIter.next();
      if ( remoteAddrIter.hasElement() )
        remoteAddrVals += ",";
      else
        break;
    }

    utilx::runtime_cfg::CompositeParameter* localAddrs =
        linkParam->getParameter<utilx::runtime_cfg::CompositeParameter>("local_addresses");
    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter>
    localAddrIter = localAddrs->getIterator<utilx::runtime_cfg::Parameter>("address");
    std::string localAddrVals;
    while( localAddrIter.hasElement() ) {
      localAddrVals += localAddrIter.getCurrentElement()->getValue();
      localAddrIter.next();
      if ( localAddrIter.hasElement() )
        localAddrVals += ",";
      else
        break;
    }

    char paramName[128];
    sprintf(paramName, "M3uaGW.sgp_links.%s.remote_addresses", linkParam->getValue().c_str());
    xml_conf->setString(paramName, remoteAddrVals.c_str());

    sprintf(paramName, "M3uaGW.sgp_links.%s.remote_port", linkParam->getValue().c_str());
    xml_conf->setInt(paramName,
                     linkParam->getParameter<utilx::runtime_cfg::Parameter>("remote_port")->getIntValue());

    sprintf(paramName, "M3uaGW.sgp_links.%s.local_addresses", linkParam->getValue().c_str());
    xml_conf->setString(paramName, localAddrVals.c_str());

    sprintf(paramName, "M3uaGW.sgp_links.%s.local_port", linkParam->getValue().c_str());
    xml_conf->setInt(paramName,
                     linkParam->getParameter<utilx::runtime_cfg::Parameter>("local_port")->getIntValue());

    linkIter.next();
  }
}

void
RuntimeConfig::preparePointsDefinition(smsc::util::config::Config* xml_conf)
{
  /**
   * config.points-definition.point = LPC_11044
   * config.points-definition.point.lpc = 11044
   * config.points-definition.point.ni = 3
   * config.points-definition.point.standard = ITU
   */
  utilx::runtime_cfg::CompositeParameter& pointsDef =
      find<utilx::runtime_cfg::CompositeParameter>("config.points-definition");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
    pointIter = pointsDef.getIterator<utilx::runtime_cfg::CompositeParameter>("point");

  while ( pointIter.hasElement() ) {
    char paramName[1024];
    utilx::runtime_cfg::CompositeParameter* point = pointIter.getCurrentElement();
    snprintf(paramName, sizeof(paramName), "M3uaGW.points-definition.%s.lpc", point->getValue().c_str());
    xml_conf->setInt(paramName,
                     point->getParameter<utilx::runtime_cfg::Parameter>("lpc")->getIntValue());
    snprintf(paramName, sizeof(paramName), "M3uaGW.points-definition.%s.ni", point->getValue().c_str());
    xml_conf->setInt(paramName,
                     point->getParameter<utilx::runtime_cfg::Parameter>("ni")->getIntValue());
    snprintf(paramName, sizeof(paramName), "M3uaGW.points-definition.%s.standard", point->getValue().c_str());
    xml_conf->setString(paramName,
                        point->getParameter<utilx::runtime_cfg::Parameter>("standard")->getValue().c_str());
    pointIter.next();
  }
}

void
RuntimeConfig::prepareMtp3RoutingTable(smsc::util::config::Config* xml_conf)
{
  /**
   * config.mtp3-routing-tables.table = LPC_133
   * config.mtp3-routing-tables.table.lpc = 133

   * config.mtp3-routing-tables.table.entry = route_entry_1
   * config.mtp3-routing-tables.table.entry.dpc = 132
   * config.mtp3-routing-tables.table.entry.sgp_link = LINK_TO_SG1

   * config.mtp3-routing-tables.table.entry = route_entry_2
   * config.mtp3-routing-tables.table.entry.dpc = 131
   * config.mtp3-routing-tables.table.entry.sgp_link = LINK_TO_SG2
   */
  utilx::runtime_cfg::CompositeParameter& mtp3RoutingTables =
      find<utilx::runtime_cfg::CompositeParameter>("config.mtp3-routing-tables");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> tableIter =
      mtp3RoutingTables.getIterator<utilx::runtime_cfg::CompositeParameter>("table");
  while ( tableIter.hasElement() ) {
    utilx::runtime_cfg::CompositeParameter* table = tableIter.getCurrentElement();
    char paramName[1024];

    snprintf(paramName, sizeof(paramName), "M3uaGW.mtp3-routing-tables.%s.lpc", table->getValue().c_str());
    xml_conf->setInt(paramName, table->getParameter<utilx::runtime_cfg::Parameter>("lpc")->getIntValue());

    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> entryIter =
        table->getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
    while ( entryIter.hasElement() ) {
      utilx::runtime_cfg::CompositeParameter* entry = entryIter.getCurrentElement();
      snprintf(paramName, sizeof(paramName), "M3uaGW.mtp3-routing-tables.%s.%s.dpc",
               table->getValue().c_str(), entry->getValue().c_str());
      xml_conf->setInt(paramName,
                       entry->getParameter<utilx::runtime_cfg::Parameter>("dpc")->getIntValue());

      snprintf(paramName, sizeof(paramName), "M3uaGW.mtp3-routing-tables.%s.%s.sgp_link",
               table->getValue().c_str(), entry->getValue().c_str());
      xml_conf->setString(paramName,
                          entry->getParameter<utilx::runtime_cfg::Parameter>("sgp_link")->getValue().c_str());

      entryIter.next();
    }
    tableIter.next();
  }
}

void
RuntimeConfig::prepareTranslationTable(smsc::util::config::Config* xml_conf)
{
  /**
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
  utilx::runtime_cfg::CompositeParameter& translationTable =
      find<utilx::runtime_cfg::CompositeParameter>("config.translation-table");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> entryIter =
      translationTable.getIterator<utilx::runtime_cfg::CompositeParameter>("entry");

  while ( entryIter.hasElement() ) {
    utilx::runtime_cfg::CompositeParameter* entry = entryIter.getCurrentElement();
    utilx::runtime_cfg::CompositeParameter* sccpUsers =
        entry->getParameter<utilx::runtime_cfg::CompositeParameter>("sccp_users");
    if ( sccpUsers ) {
      utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> userIdIter =
          sccpUsers->getIterator<utilx::runtime_cfg::Parameter>("userid");
      std::string userIds;
      while ( userIdIter.hasElement() ) {
        userIds += userIdIter.getCurrentElement()->getValue();
        userIdIter.next();
        if ( userIdIter.hasElement() )
          userIds += ",";
        else
          break;
      }

      char paramName[1024];
      snprintf(paramName, sizeof(paramName), "M3uaGW.translation-table.%s.gt",
               entry->getValue().c_str());
      xml_conf->setString(paramName,
                          entry->getParameter<utilx::runtime_cfg::Parameter>("gt")->getValue().c_str());

      snprintf(paramName, sizeof(paramName), "M3uaGW.translation-table.%s.sccp_users",
               entry->getValue().c_str());
      xml_conf->setString(paramName, userIds.c_str());

      snprintf(paramName, sizeof(paramName), "M3uaGW.translation-table.%s.traffic-mode",
               entry->getValue().c_str());
      xml_conf->setString(paramName,
                          entry->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode")->getValue().c_str());
    } else {
      utilx::runtime_cfg::CompositeParameter* dpcsParam =
          entry->getParameter<utilx::runtime_cfg::CompositeParameter>("dpcs");
      utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> pcIter =
          dpcsParam->getIterator<utilx::runtime_cfg::Parameter>("pc");
      std::string dpcs;
      while ( pcIter.hasElement() ) {
        dpcs += pcIter.getCurrentElement()->getValue();
        pcIter.next();
        if ( pcIter.hasElement() )
          dpcs += ",";
        else
          break;
      }
      char paramName[1024];
      snprintf(paramName, sizeof(paramName), "M3uaGW.translation-table.%s.gt",
               entry->getValue().c_str());
      xml_conf->setString(paramName,
                          entry->getParameter<utilx::runtime_cfg::Parameter>("gt")->getValue().c_str());

      snprintf(paramName, sizeof(paramName), "M3uaGW.translation-table.%s.dpcs",
               entry->getValue().c_str());
      xml_conf->setString(paramName, dpcs.c_str());

      snprintf(paramName, sizeof(paramName), "M3uaGW.translation-table.%s.lpc",
               entry->getValue().c_str());
      xml_conf->setInt(paramName,
                       entry->getParameter<utilx::runtime_cfg::Parameter>("lpc")->getIntValue());

      snprintf(paramName, sizeof(paramName), "M3uaGW.translation-table.%s.traffic-mode",
               entry->getValue().c_str());
      xml_conf->setString(paramName,
                          entry->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode")->getValue().c_str());

    }
    entryIter.next();
  }
}

void
RuntimeConfig::saveXmlConfig(smsc::util::config::Config& xml_conf)
{
  char backupCfgFname[1024];
  tm lcltm;
  char timeStr[64];
  timeval tp;
  gettimeofday(&tp, 0);
  strftime(timeStr, sizeof(timeStr), "%d%m%Y%H%M%S", localtime_r(&tp.tv_sec, &lcltm));

  snprintf(backupCfgFname, sizeof(backupCfgFname), "%s.%s", _cfgFile, timeStr);
  if ( rename(_cfgFile, backupCfgFname) < 0 )
    throw smsc::util::SystemError("RuntimeConfig::commit::: can't backup config file");
  xml_conf.saveToFile(_cfgFile);
}

}}}}
