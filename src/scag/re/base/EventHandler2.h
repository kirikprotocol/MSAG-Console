#ifndef _EVENT_HANDLER2_H_
#define _EVENT_HANDLER2_H_

#include <list>
#include <logger/Logger.h>

#include "ActionFactory2.h"
#include "LongCallAction2.h"

namespace scag2 {

namespace sessions {
class SessionPrimaryKey;
}

namespace util {
class HRTiming;
}

namespace re {

using transport::SCAGCommand;
using namespace actions;
using smsc::logger::Logger;
// using namespace lcm;
using namespace sessions;

class EventHandler : public IParserHandler, ComplexActionLongCallHelper
{
    EventHandler(const EventHandler &);
protected:
    PropertyObject propertyObject;
    Logger * logger;
    std::vector<Action *> actions;
//////////////IParserHandler Interfase///////////////////////
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
//////////////IParserHandler Interfase///////////////////////
    void RunActions(ActionContext& context);
    void RegisterTrafficEvent( const CommandProperty& commandProperty,
                               const SessionPrimaryKey& sessionPrimaryKey,
                               const std::string& messageBody,
                               const std::string* keywords );
    
    void RegisterAlarmEvent(uint32_t eventId, const std::string& addr, uint8_t protocol,
                            uint32_t serviceId, uint32_t providerId, uint32_t operatorId,
                            uint16_t commandStatus, const SessionPrimaryKey& sessionPrimaryKey,
                            char dir);
public:
    EventHandler() :logger(0)  { logger = Logger::getInstance("scag.re"); };
    virtual ~EventHandler();

    virtual void init(const SectionParams& params,PropertyObject _propertyObject) {propertyObject = _propertyObject;}
    virtual void process( SCAGCommand& command,Session& session, RuleStatus& rs, util::HRTiming* hrt = 0 ) = 0;
    virtual int StrToHandlerId(const std::string& str) = 0;

};

}}
#endif
