#include "InfoSmeMessageSender.h"

namespace smsc {
namespace infosme {

InfoSmeMessageSender::InfoSmeMessageSender( TaskProcessor& processor ) :
processor_(processor)
{}


InfoSmeMessageSender::~InfoSmeMessageSender()
{
    ConnectorIterator it = connectors_.getIterator();
    SmscConnector* connector;
    char *key = 0;
    while(it.Next(key, connector)) {
        if (connector) {
            delete connector;
        }
    }
}


void InfoSmeMessageSender::start()
{
    defaultConnector_->Start();

    ConnectorIterator it = connectors_.getIterator();
    SmscConnector* connector;
    char *key = 0;
    while(it.Next(key, connector)) {
        if (connector) {
            connector->Start();
        }
    }
}


void InfoSmeMessageSender::stop() 
{
    defaultConnector_->stop();
    ConnectorIterator it = connectors_.getIterator();
    SmscConnector* connector;
    char *key = 0;
    while(it.Next(key, connector)) {
        if (connector) {
            connector->stop();
        }
    }
}


SmscConnector* InfoSmeMessageSender::getSmscConnector( const string& regionId )
{
    const string* smscId = regions_.GetPtr(regionId.c_str());
    if (!smscId) {
        smsc_log_debug(logger, "SMSC id for region '%s' not set. Default SMSC '%s' will be used.", regionId.c_str(), defaultSmscId_.c_str());
        return defaultConnector_.get();
    }
    SmscConnector** connector = connectors_.GetPtr(smscId->c_str());
    if (connector) {
        smsc_log_debug(logger, "SMSC id='%s' will be used for region '%s'.",smscId->c_str(), regionId.c_str());
        return *connector;
    }
    smsc_log_warn(logger, "SMSC connector id='%s' for region '%s' not found. Default SMSC '%s' will be used.",
                  smscId->c_str(), regionId.c_str(), defaultSmscId_.c_str());
    return defaultConnector_.get();
}


void InfoSmeMessageSender::reloadSmscAndRegions( Manager& manager )
{
    const std::string csn("InfoSme.SMSCConnectors");
    ConfigView ccv(manager,csn.c_str());

    // get the default connector id
    const std::string defId = ccv.getString("default", "default SMSC Connector id not set");

    // create connectors
    std::auto_ptr<CStrSet> connNames(ccv.getShortSectionNames());
    // FIXME: delete connectors that are not found in new set
    defaultConnector_ = 0;
    ...;
    for ( CStrSet::iterator i = connNames.get()->begin(); i != connNames.get()->end(); ++i ) {
        const std::string sectName( csn + "." + *i );
        InfoSmeConfig cfg(ConfigView(manager,sectName.c_str()));
        SmscConnector* conn = addConnector(cfg,*i);
        if ( *i == defId ) {
            defaultConnector_ = conn;
        }
    }
    if ( !defaultConnector_ ) {
        throw ConfigException("the section for default SMSC Connector is not found");
    }

    // delete all regions-to-smsc mapping
    regions_.Empty();

    // regions
    ConfigView tpConfig(manager,"InfoSme");
    const char* route_xml_file = tpConfig.getString("route_config_filename");
    const char* regions_xml_file = tpConfig.getString("regions_config_filename");
    regionsConfig_.reset(new smsc::util::config::region::RegionsConfig(regions_xml_file));
    smsc::util::config::region::RegionsConfig::status st = regionsConfig_->load();
    if ( st == smsc::util::config::region::RegionsConfig::success )
        smsc_log_info(log, "config file %s has been loaded successful", regions_xml_file);
    else
        throw smsc::util::config::ConfigException("can't load config file %s", regions_xml_file);

    smsc::util::config::route::RouteConfig routeConfig;
    if ( routeConfig.load(route_xml_file) == smsc::util::config::route::RouteConfig::success )
        smsc_log_info(log, "config file %s has been loaded successful", route_xml_file);
    else
        throw smsc::util::config::ConfigException("can't load config file %s", route_xml_file);

    smsc::util::config::region::Region* region;
    smsc::util::config::region::RegionsConfig::RegionsIterator regsIter = regionsConfig->getIterator();
    while (regsIter.fetchNext(region) == smsc::util::config::region::RegionsConfig::success) {
        region->expandSubjectRefs(routeConfig);
        smsc::util::config::region::Region::MasksIterator maskIter = region->getMasksIterator();
        std::string addressMask;
        while(maskIter.fetchNext(addressMask)) {
            smsc::util::config::region::RegionFinder::getInstance().registerAddressMask(addressMask, region);
        }
        addRegionMapping( region->getId(), region->getInfosmeSmscId() );
    }
    smsc::util::config::region::RegionFinder::getInstance().registerDefaultRegion(&(regionsConfig->getDefaultRegion()));
}

} // namespace infosme
} // namespace smsc
