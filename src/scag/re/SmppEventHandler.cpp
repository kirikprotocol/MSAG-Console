#include "SmppEventHandler.h"
#include "SAX2Print.hpp"

namespace scag { namespace re {

RuleStatus SmppEventHandler::process(SCAGCommand command)
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

void SmppEventHandler::init(const SectionParams& params)
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
