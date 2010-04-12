#ifndef INFOSME_INFOSMEMESSAGESENDER_H
#define INFOSME_INFOSMEMESSAGESENDER_H

#include <string>
#include "MessageSender.h"
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

    virtual void reloadSmscAndRegions( Manager& mgr );
    virtual bool send( Task* task, Message& message );
    virtual void processWaitingEvents(time_t tm);

private:

    virtual SmscConnector* getSmscConnector(const std::string& regionId);
    SmscConnector* addConnector( const smsc::sme::SmeConfig& cfg,
                                 const std::string& smscid,
                                 bool               perftest = false );
    void addRegionMapping( const std::string& regionId, const std::string& smscId );

private:
    smsc::logger::Logger* log_;
    TaskProcessor&        processor_;
    std::auto_ptr< smsc::util::config::region::RegionsConfig > regionsConfig_;
    SmscConnector*        defaultConnector_;
    Hash<SmscConnector*>  connectors_;  // owned, all connectors
    Hash<std::string>     regions_;
    bool                  started_;
};

} // namespace infosme
} // namespace smsc

#endif /* !INFOSME_INFOSMEMESSAGESENDER_H */
