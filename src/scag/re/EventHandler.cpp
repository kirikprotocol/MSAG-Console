#include "EventHandler.h"
//#include <util/Exception.hpp>

namespace scag { namespace re {

using namespace scag::util::properties;


EventHandler::~EventHandler()
{
    //smsc_log_debug(logger, "EventHandler: start destructor");
    int key;
    Action * value;

    std::list<Action *>::const_iterator it;

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
    std::list<Action *>::const_iterator it;

    //TODO: Fill default rs fields
    RuleStatus rs;

    rs.result = 1;
    rs.status = true;

    context.setRuleStatus(rs);

    for (it = actions.begin(); it!=actions.end(); ++it)
    {
        if (!(*it)->run(context)) break;
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

    std::string tempStr = commandProperty.abonentAddr.toString();

    if (tempStr.size() > MAX_ABONENT_NUMBER_LENGTH - 1)
    {
        memcpy((char *)ev.Header.pAbonentNumber,str, MAX_ABONENT_NUMBER_LENGTH - 1);
        ev.Header.pAbonentNumber[MAX_ABONENT_NUMBER_LENGTH] = 0;
    }
    else
        sprintf((char *)ev.Header.pAbonentNumber,"%s",tempStr.c_str());

    ev.Header.sCommandStatus = commandProperty.status;
    ev.iOperatorId = commandProperty.operatorId;
    
    if ((propertyObject.HandlerId == EH_SUBMIT_SM)||(propertyObject.HandlerId == EH_DELIVER_SM))
    {
        int size = (MAX_TEXT_MESSAGE_LENGTH - 1) * sizeof(uint16_t);
        if (size > messageBody.size()) size = messageBody.size();

        if (size > 0) 
        {
            memcpy(ev.pMessageText, messageBody.data(), size); 
            ev.pMessageText[MAX_TEXT_MESSAGE_LENGTH] = 0;
        }
    }

    tempStr = sessionPrimaryKey.abonentAddr.toString();

    sprintf((char *)ev.pSessionKey,"%s/%ld%d", tempStr.c_str(), sessionPrimaryKey.BornMicrotime.tv_sec,sessionPrimaryKey.BornMicrotime.tv_usec / 1000);

    if ((propertyObject.HandlerId == EH_DELIVER_SM)||(propertyObject.HandlerId == EH_SUBMIT_SM_RESP))
        ev.cDirection = 'I';
    else
        ev.cDirection = 'O';

    Statistics::Instance().registerSaccEvent(ev);
}

}}
