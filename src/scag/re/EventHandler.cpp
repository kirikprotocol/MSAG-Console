#include "EventHandler.h"
#include "MainActionFactory.h"
#include <util/Exception.hpp>
#include "SAX2Print.hpp"


namespace scag { namespace re {

using namespace scag::util::properties;


EventHandler::~EventHandler()
{
    int key;
    Action * value;

    for (IntHash<Action *>::Iterator it = actions.First(); it.Next(key, value);)
    {
        delete value;
    }
    smsc_log_debug(logger, "EventHandler released");
}

RuleStatus EventHandler::process(SCAGCommand command)
{
    int key;
    Action * action;
    Hash<Property> _constants;
    PropertyManager _session;
    PropertyManager _command;
    ////////////////////
    Property p;
    p.setStr("25");
    _constants.Insert("const",p);

    //////////////////////

    ActionContext context(_constants, _session, _command);

    smsc_log_debug(logger, "Process EventHandler...");
    for (IntHash<Action *>::Iterator it = actions.First(); it.Next(key, action);)
    {
        if (!action->run(context)) break;
    }
    RuleStatus rs;
    rs = context.getStatus();
    return rs;
}

HandlerType EventHandler::StrToHandlerType(const std::string& str)
{
    HandlerType result = htUnknown;

    if (str.compare("DELIVER")==0) result = htDeliver;
    if (str.compare("SUBMIT")==0)  result = htSubmit;



    return result;
}


//////////////IParserHandler Interfase///////////////////////

IParserHandler * EventHandler::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{

    Action * action = 0;
    action = factory.CreateAction(name);
    if (!action) 
    {
        std::string msg("EventHandler: unrecognized child object '");
        msg.append(name);
        msg.append("' to create");
        throw Exception(msg.c_str());
    }

    smsc_log_debug(logger,std::string("HandlerEvent::StartXMLSubSection - <")+name+std::string(">"));

    try
    {
        action->init(params);
    } catch (Exception& e)
    {
        delete action;
        throw e;
    }

    actions.Insert(actions.Count(),action);
    return action;
}

bool EventHandler::FinishXMLSubSection(const std::string& name)
{
    return (name.compare("handler")==0);
}


void EventHandler::init(const SectionParams& params)
{
    if (!params.Exists("type")) throw Exception("EventHandler: missing 'type' parameter");

    std::string sHandlerType = params["type"];
    handlerType = StrToHandlerType(sHandlerType);
    if (handlerType==htUnknown) 
    {
        std::string msg("EventHandler: invalid value '") ;
        msg.append(sHandlerType);
        msg.append("' for 'type' parameter");
        throw Exception(msg.c_str());
    }
    smsc_log_debug(logger,"HandlerEvent::Init");
}



}}
