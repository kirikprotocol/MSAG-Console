#include "EventHandler.h"
#include "scag/lcm/LongCallManager.h"
//#include <util/Exception.hpp>

namespace scag { namespace re {

using namespace scag::util::properties;
using namespace scag::lcm;


EventHandler::~EventHandler()
{
    //smsc_log_debug(logger, "EventHandler: start destructor");
    int key;
    Action * value;

    std::vector<Action *>::const_iterator it;

    for (it = actions.begin(); it!=actions.end(); ++it)
    {
        delete (*it);
    }
    //smsc_log_debug(logger, "EventHandler released");
}



//////////////IParserHandler Interfase///////////////////////

IParserHandler * EventHandler::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{

    Action * action = 0;
    action = factory.CreateAction(name);
    if (!action) 
        throw SCAGException("EventHandler: unrecognized child object '%s' to create",name.c_str());

    smsc_log_debug(logger,"HandlerEvent::StartXMLSubSection - <%s>",name.c_str());

    try
    {
        action->init(params,propertyObject);
    } catch (SCAGException& e)
    {
        delete action;
        throw e;
    }

    actions.push_back(action);
    return action;
}

bool EventHandler::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void EventHandler::RunActions(ActionContext& context)
{
    context.getRuleStatus().status = STATUS_OK;

    LongCallContext& longCallContext = context.getSession().getLongCallContext();

    ActionStackValue sv(0, false);
    longCallContext.ActionStack.push(sv);

    RunActionVector(context, longCallContext, actions, logger);
}

void EventHandler::RegisterTrafficEvent(const CommandProperty& commandProperty, const CSessionPrimaryKey& sessionPrimaryKey, const std::string& messageBody)
{
    SaccTrafficInfoEvent* ev = new SaccTrafficInfoEvent();

    ev->Header.cCommandId = propertyObject.HandlerId;
    ev->Header.cProtocolId = commandProperty.protocol;
    ev->Header.iServiceId = commandProperty.serviceId;
    ev->Header.iServiceProviderId = commandProperty.providerId; 
    
    timeval tv;
    gettimeofday(&tv,0);

    ev->Header.lDateTime = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    ev->Header.pAbonentNumber = commandProperty.abonentAddr.toString();
    ev->Header.sCommandStatus = commandProperty.status;
    ev->Header.iOperatorId = commandProperty.operatorId;
    
    if ((propertyObject.HandlerId == EH_SUBMIT_SM)||(propertyObject.HandlerId == EH_DELIVER_SM)||(propertyObject.HandlerId == EH_DATA_SM))
        ev->pMessageText.append(messageBody.data(), messageBody.size());

    sessionPrimaryKey.toString(ev->Header.pSessionKey);

    if ((commandProperty.direction == dsdSc2Srv) || (propertyObject.HandlerId == EH_HTTP_REQUEST) || (propertyObject.HandlerId == EH_HTTP_DELIVERY))
        ev->cDirection = 'I';
    else
        ev->cDirection = 'O';

    Statistics::Instance().registerSaccEvent(ev);
}

void EventHandler::RegisterAlarmEvent(uint32_t eventId, const std::string& addr, uint8_t protocol, uint32_t serviceId, uint32_t providerId, uint32_t operatorId, uint16_t commandStatus, const CSessionPrimaryKey& sessionPrimaryKey, char dir)
{
    SaccAlarmEvent* ev = new SaccAlarmEvent();

    ev->Header.cCommandId = 3;
    ev->Header.cProtocolId = protocol;
    ev->Header.iServiceId = serviceId;
    ev->Header.iServiceProviderId = providerId; 
    ev->Header.pAbonentNumber = addr;
    ev->Header.sCommandStatus = commandStatus;
    ev->Header.iOperatorId = operatorId;
    ev->iAlarmEventId = eventId;
    
    timeval tv;
    gettimeofday(&tv,0);
    ev->Header.lDateTime = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    sessionPrimaryKey.toString(ev->Header.pSessionKey);
    ev->cDirection = dir;

    Statistics::Instance().registerSaccEvent(ev);
}

}}
