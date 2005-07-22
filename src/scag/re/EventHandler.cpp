#include "scag/re/EventHandler.h"


namespace scag { namespace re {



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
    p.setStr("5");
    _constants["const"] = p;

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
}



void EventHandler::SetChildObject(IParserHandler * child)
{
    if (!child) return;

    Action * action = dynamic_cast<Action *>(child);
    if (!action) return throw Exception("Event Handler: unrecognized child object");

    smsc_log_debug(logger, "Set child object to EventHandler");
    actions.Insert(actions.Count(),action);
}



}}
