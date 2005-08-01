#include "ActionTrafficCheck.h"
#include "scag/re/ActionFactory.h"
#include "scag/re/SAX2Print.hpp"

namespace scag { namespace re { namespace actions {


IParserHandler * ActionTrafficCheck::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    Action * action = 0;
    action = factory.CreateAction(name);
    if (!action) 
    {
        std::string msg("Action 'traffic:check': unrecognized child object '");
        msg.append(name);
        msg.append("' to create");
        throw Exception(msg.c_str());
    }

    try
    {
        action->init(params);
    } catch (Exception& e)
    {
        delete action;
        throw e;
    }
    Actions.push_back(action);
    return action;
}


bool ActionTrafficCheck::FinishXMLSubSection(const std::string& name)
{
    return (name == "traffic:check");
}

void ActionTrafficCheck::init(const SectionParams& params)
{
    if (!params.Exists("max")) throw Exception("Action 'traffic:check': missing 'max' parameter");
    if (!params.Exists("period")) throw Exception("Action 'traffic:check': missing 'period' parameter");

    sPeriod = params["period"];
    sMax = params["max"];
}

bool ActionTrafficCheck::run(ActionContext& context)
{
    int key;
    Action * value;

    std::list<Action *>::const_iterator it;

    for (it = Actions.begin(); it!=Actions.end(); ++it)
    {
        if (!(*it)->run(context)) return false;
    }
    return true;
}

ActionTrafficCheck::~ActionTrafficCheck()
{
    int key;
    Action * value;

    std::list<Action *>::const_iterator it;

    for (it = Actions.begin(); it!=Actions.end(); ++it)
    {
        delete (*it);
    }

    smsc_log_debug(logger, "'TrafficCheck' action released");
}


}}}