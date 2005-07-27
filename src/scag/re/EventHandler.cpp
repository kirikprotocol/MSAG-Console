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




}}
