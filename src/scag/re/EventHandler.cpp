#include "EventHandler.h"
//#include <util/Exception.hpp>

namespace scag { namespace re {

using namespace scag::util::properties;


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

RuleStatus EventHandler::RunActions(ActionContext& context)
{
    RuleStatus rs;

    rs.status = STATUS_OK;

    context.setRuleStatus(rs);

    int startIndex = 0;
    if (!context.ActionStack.empty()) 
    {
        startIndex = context.ActionStack.top().actionIndex;
        if (startIndex >= actions.size())
        {
            smsc_log_error(logger, "Cannot continue running actions. Details: action index out of bound");
            while (!context.ActionStack.empty()) context.ActionStack.pop();
            return context.getRuleStatus();
        }
        context.ActionStack.pop();
    }

    for (int i = startIndex; i < actions.size(); i++) 
    {
        if (!(actions[i]->run(context))) 
        {
            ActionStackValue sv(i, false);
            context.ActionStack.push(sv);
            break;
        }
    }
 
    return context.getRuleStatus();
}

void EventHandler::RegisterTrafficEvent(const CommandProperty& commandProperty, const CSessionPrimaryKey& sessionPrimaryKey, const std::string& messageBody)
{
    SACC_TRAFFIC_INFO_EVENT_t ev;

    ev.Header.cCommandId = propertyObject.HandlerId;

    ev.Header.cProtocolId = commandProperty.protocol;

    ev.Header.iServiceId = commandProperty.serviceId;
    ev.Header.iServiceProviderId = commandProperty.providerId; 
    
    timeval tv;
    gettimeofday(&tv,0);

    ev.Header.lDateTime = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;


    ev.Header.pAbonentNumber = commandProperty.abonentAddr.toString();

    ev.Header.sCommandStatus = commandProperty.status;
    ev.Header.iOperatorId = commandProperty.operatorId;
    
    if ((propertyObject.HandlerId == EH_SUBMIT_SM)||(propertyObject.HandlerId == EH_DELIVER_SM)||(propertyObject.HandlerId == EH_DATA_SM))
    {

        ev.pMessageText.append(messageBody.data(), messageBody.size());
    }

    sessionPrimaryKey.toString(ev.pSessionKey);

    if ((propertyObject.HandlerId == EH_DELIVER_SM)||(propertyObject.HandlerId == EH_SUBMIT_SM_RESP) || (propertyObject.HandlerId == EH_HTTP_REQUEST))
        ev.cDirection = 'I';
    else
        ev.cDirection = 'O';

    Statistics::Instance().registerSaccEvent(ev);
}

void EventHandler::RegisterAlarmEvent(uint32_t eventId, const std::string& addr, uint8_t protocol, uint32_t serviceId, uint32_t providerId, uint32_t operatorId, uint16_t commandStatus, const std::string& sessionPrimaryKey, char dir)
{
    SACC_ALARM_t ev;

    ev.Header.cCommandId = 3;
    ev.Header.cProtocolId = protocol;
    ev.Header.iServiceId = serviceId;
    ev.Header.iServiceProviderId = providerId; 
    ev.Header.pAbonentNumber = addr;
    ev.Header.sCommandStatus = commandStatus;
    ev.Header.iOperatorId = operatorId;
    ev.iAlarmEventId = eventId;
    
    timeval tv;
    gettimeofday(&tv,0);
    ev.Header.lDateTime = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    ev.pSessionKey = sessionPrimaryKey;
    ev.cDirection = dir;

    Statistics::Instance().registerSaccEvent(ev);
}

}}
