#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_

#include <list>
#include <logger/Logger.h>


#include "scag/re/actions/ActionFactory.h"
#include "scag/lcm/LongCallManager.h"
#include "scag/re/actions/LongCallAction.h"

namespace scag { namespace re
{
using scag::transport::SCAGCommand;
using namespace scag::re::actions;
using smsc::logger::Logger;
using namespace scag::lcm;


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
    void RegisterTrafficEvent(const CommandProperty& commandProperty, const CSessionPrimaryKey& sessionPrimaryKey, const std::string& messageBody);
    
    void RegisterAlarmEvent(uint32_t eventId, const std::string& addr, uint8_t protocol,
                            uint32_t serviceId, uint32_t providerId, uint32_t operatorId,
                            uint16_t commandStatus, const CSessionPrimaryKey& sessionPrimaryKey, char dir);
public:
    EventHandler() :logger(0)  { logger = Logger::getInstance("scag.re"); };
    virtual ~EventHandler();

    virtual void init(const SectionParams& params,PropertyObject _propertyObject) {propertyObject = _propertyObject;}
    virtual void process(SCAGCommand& command,Session& session, RuleStatus& rs) = 0;
    virtual int StrToHandlerId(const std::string& str) = 0;

};

}}
#endif
