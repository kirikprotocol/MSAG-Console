#ifndef INFOSME_MESSAGESENDER_H
#define INFOSME_MESSAGESENDER_H

#include <string>
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

namespace smsc {
namespace infosme {

class SmscConnector;
class Task;
class TaskProcessor;
class Message;

struct MessageSender
{
    virtual uint32_t sendSms(const std::string& src,const std::string& dst,const std::string& txt,bool flash)=0;
    virtual ~MessageSender() {};

    /// NOTE: both methods are externally (in taskprocessor) guarded 
    /// against connector/regions list modifications
    // virtual void reloadSmscAndRegions( smsc::util::config::Manager& mgr ) = 0;
    // virtual bool send( Task* task, Message& message ) = 0;
    virtual void processWaitingEvents(time_t tm) = 0;
    virtual unsigned send( unsigned deltaTime, unsigned sleepTime ) = 0;
    virtual void init( TaskProcessor&, smsc::util::config::ConfigView* ) = 0;

protected:
    MessageSender() {}
};

} // namespace infosme
} // namespace smsc

#endif /* !INFOSME_MESSAGESENDER_H */
