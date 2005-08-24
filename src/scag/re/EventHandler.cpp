#include "EventHandler.h"
#include <util/Exception.hpp>
#include "SAX2Print.hpp"

namespace scag { namespace re {

using namespace scag::util::properties;


EventHandler::~EventHandler()
{
    int key;
    Action * value;

    std::list<Action *>::const_iterator it;

    for (it = actions.begin(); it!=actions.end(); ++it)
    {
        delete (*it);
    }
    smsc_log_debug(logger, "EventHandler released");
}



//////////////IParserHandler Interfase///////////////////////

IParserHandler * EventHandler::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{

    Action * action = 0;
    action = factory.CreateAction(name);
    if (!action) 
        throw RuleEngineException("EventHandler: unrecognized child object '",name.c_str(),"' to create");

    smsc_log_debug(logger,std::string("HandlerEvent::StartXMLSubSection - <")+name+std::string(">"));

    try
    {
        action->init(params,propertyObject);
    } catch (Exception& e)
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




}}
