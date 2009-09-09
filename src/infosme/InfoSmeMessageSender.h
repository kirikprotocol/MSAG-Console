#ifndef INFOSME_INFOSMEMESSAGESENDER_H
#define INFOSME_INFOSMEMESSAGESENDER_H

#include <string>
#include "sme/SmppBase.hpp"
#include "util/config/region/RegionsConfig.hpp"
#include "logger/Logger.h"
#include "core/buffers/Hash.hpp"
#include "TaskProcessor.h"

namespace smsc {
namespace infosme {

class SmscConnector;
class TaskProcessor;

class InfoSmeMessageSender : public MessageSender
{
private:
    typedef Hash<SmscConnector*>::Iterator ConnectorIterator;

public:
    InfoSmeMessageSender( TaskProcessor& processor );

    virtual ~InfoSmeMessageSender();
    void start();
    void stop();

    virtual uint32_t sendSms(const std::string& org,const std::string& dst,const std::string& txt,bool flash);
    /*
    {
        smsc_log_info(logger, "sendSms do default region!");
        return defaultConnector_->sendSms(org,dst,txt,flash);
    }
     */

    virtual SmscConnector* getSmscConnector(const std::string& regionId);
    virtual void reloadSmscAndRegions( Manager& mgr );

private:

    SmscConnector* addConnector( const smsc::sme::SmeConfig& cfg, const std::string& smscid );
    void addRegionMapping( const std::string& regionId, const std::string& smscId );

    /*
    void addConnector(const InfoSmeConfig& cfg, const string& smscId) {
      if (smscId == defaultSmscId_) {
        //throw ConfigException("SMSC Connector with id='%s' is default and already exists.", smscId.c_str());
        smsc_log_debug(logger, "SMSC Connector with id='%s' is default and already exists.", smscId.c_str());
        return;
      }
      if (connectors_.Exists(smscId.c_str())) {
        throw ConfigException("SMSC Connector id='%s' is not unique.", smscId.c_str());
      }
      connectors_.Insert(smscId.c_str(), new SmscConnector(processor_, cfg, smscId));
    }

    void addConnectors(Manager& manager, ConfigView& config, const std::string& sectinoName) {

      std::auto_ptr<CStrSet> connectors(config.getShortSectionNames());

      for (CStrSet::iterator i = connectors.get()->begin(); i != connectors.get()->end(); ++i) 
      {
          std::string connectorSectionName = sectinoName + "." + (*i); 
          ConfigView connectorCfgView(manager, connectorSectionName.c_str());
  
          InfoSmeConfig connectorCfg(ConfigView(manager, connectorSectionName.c_str()));
          addConnector(connectorCfg, *i);
      }
    }


    void addRegion(const string& regionId, const string& smscId) {
      if (smscId.empty() || smscId == defaultSmscId_) {
        smsc_log_info(logger, "SMSC id '%s' for region '%s' set. Default SMSC Connector '%s' will be used.", smscId.c_str(), regionId.c_str(), defaultSmscId_.c_str());
        regions_.Insert(regionId.c_str(), defaultSmscId_);
        return;
      }
      if (regions_.Exists(regionId.c_str())) {
        throw ConfigException("Region already exists: '%s'", smscId.c_str());
      }
      if (!connectors_.Exists(smscId.c_str())) {
        smsc_log_info(logger, "SMSC Connector '%s' for region '%s' unknown. Default SMSC Connector '%s' will be used.",
                      smscId.c_str(), regionId.c_str(), defaultSmscId_.c_str());
        regions_.Insert(regionId.c_str(), defaultSmscId_);
        return;
      }
      smsc_log_info(logger, "SMSC Connector '%s' for region '%s' will be used.", smscId.c_str(), regionId.c_str());
      regions_.Insert(regionId.c_str(), smscId);
    }
     */

private:
    std::auto_ptr< smsc::util::config::region::RegionsConfig > regionsConfig_;
    smsc::logger::Logger* log_;
    TaskProcessor&        processor_;
    SmscConnector*        defaultConnector_;
    Hash<SmscConnector*>  connectors_;  // owned, all connectors
    Hash<std::string>     regions_;
};

} // namespace infosme
} // namespace smsc

#endif /* !INFOSME_INFOSMEMESSAGESENDER_H */
