#ifndef INFOSME_MESSAGESENDER_H
#define INFOSME_MESSAGESENDER_H

#include <string>
#include "util/config/Manager.h"

namespace smsc {
namespace infosme {

class SmscConnector;
class Task;
class Message;

struct MessageSender
{
    virtual uint32_t sendSms(const std::string& src,const std::string& dst,const std::string& txt,bool flash)=0;
    virtual ~MessageSender() {};

    /// NOTE: both methods are externally (in taskprocessor) guarded 
    /// against connector/regions list modifications
    virtual SmscConnector* getSmscConnector(const std::string& regionId) = 0;
    virtual void reloadSmscAndRegions( smsc::util::config::Manager& mgr ) = 0;
    virtual bool send( Task* task, Message& message ) = 0;
protected:
    MessageSender() {}
};

} // namespace infosme
} // namespace smsc

#endif /* !INFOSME_MESSAGESENDER_H */
