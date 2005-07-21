#include "Rule.h"

namespace scag { namespace re 
{

using smsc::core::buffers::IntHash;
using namespace scag::re::actions;


/////////////////////////////ActionFactory/////////////////////////

Action * ActionFactory::CreateAction(const std::string& name) const
{
    if (name=="set") return new ActionSet();
    if (name=="if")  return new ActionIf();
    if (name=="return") return new ActionReturn();

    return 0;
}

void ActionFactory::FillTagHash(smsc::core::buffers::Hash<int>& TagHash) const
{
    TagHash["set"] = tgActionSection;
    TagHash["if"] = tgActionSection;
    TagHash["return"] = tgActionSection;
}


/////////////////////////////Rule///////////////////////////////////


Rule::~Rule()
{
    int key;
    EventHandler * value;

    for (IntHash <EventHandler *>::Iterator it = Handlers.First(); it.Next(key, value);)
    {
        delete value;
    }

    smsc_log_debug(logger,"Rule released");
}


RuleStatus Rule::process(SCAGCommand command)
{
    RuleStatus rs;
    int key;
    EventHandler * eh;

    smsc_log_debug(logger,"Process Rule...");

    for (IntHash<EventHandler *>::Iterator it = Handlers.First(); it.Next(key, eh);)
    {
        rs = eh->process(command);
        break;
    }

    return rs;
}


//////////////IParserHandler Interfase///////////////////////

void Rule::SetChildObject(IParserHandler * child)
{
    if (!child) return;

    EventHandler * eh = dynamic_cast<EventHandler *>(child);
    if (!eh) return throw Exception("Rule: unrecognized child object");

    Handlers.Insert(Handlers.Count(),eh);
}

//////////////IParserHandler Interfase///////////////////////

}}
