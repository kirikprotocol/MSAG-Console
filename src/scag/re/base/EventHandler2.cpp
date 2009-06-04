#include "EventHandler2.h"
#include "scag/util/HRTimer.h"

namespace scag2 {
namespace re {

using namespace util::properties;
using namespace lcm;

EventHandler::~EventHandler()
{
    //smsc_log_debug(logger, "EventHandler: start destructor");
    // int key;
    // Action * value;

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

    Action * action = factory.CreateAction(name);
    if (!action) 
        throw SCAGException("EventHandler: unrecognized child object '%s' to create",name.c_str());

    smsc_log_debug(logger,"HandlerEvent::StartXMLSubSection - <%s>",name.c_str());

    try
    {
        action->init(params,propertyObject);
    } catch (SCAGException& e)
    {
        delete action;
        throw;
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

}}
