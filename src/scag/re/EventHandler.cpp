#include "scag/re/EventHandler.h"


namespace scag { namespace re {


//////////////IParserHandler Interfase///////////////////////



EventHandler::EventHandler(const SectionParams& params)
{

}

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

void EventHandler::SetChildObject(IParserHandler * child)
{
    if (!child) return;

    Action * action = dynamic_cast<Action *>(child);
    if (!action) return throw Exception("Event Handler: unrecognized child object");

    smsc_log_debug(logger, "Set child object to EventHandler");
    actions.Insert(actions.Count(),action);
}

RuleStatus EventHandler::process(SCAGCommand command)
{
    int key;
    Action * action;
    Hash<Property> _constants;
    PropertyManager _session;
    PropertyManager _command;

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



}}
