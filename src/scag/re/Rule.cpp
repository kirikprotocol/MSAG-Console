#include "Rule.h"

namespace scag { namespace re 
{

using smsc::core::buffers::IntHash;
using namespace scag::re::actions;


/////////////////////////////ActionFactory/////////////////////////

Action * ActionFactory::CreateAction(const std::string& name,const SectionParams& params) const
{
    if (name=="set") return new ActionSet(params);
    if (name=="if")  return new ActionIf(params);
    if (name=="choose")  return new ActionChoose(params);
    if (name=="return")  return new ActionReturn(params);

    return 0;
}

void ActionFactory::FillTagHash(smsc::core::buffers::Hash<int>& TagHash) const
{
    TagHash["set"] = tgActionSection;
    TagHash["if"] = tgActionSection;
    TagHash["choose"] = tgActionSection;
    TagHash["return"] = tgActionSection;
}


/////////////////////////////Rule///////////////////////////////////



Rule::Rule(const SectionParams& params): useCounter(1)
{
}

Rule::~Rule()
{
    int key;
    EventHandler * value;

    for (IntHash <EventHandler *>::Iterator it = Handlers.First(); it.Next(key, value);)
    {
        delete value;
    }

    cout << "Rule released" << endl;
}


//////////////IParserHandler Interfase///////////////////////

bool Rule::SetChildObject(const IParserHandler * child)
{
    if (!child) return false;

    IParserHandler * _child = const_cast<IParserHandler *>(child);
    EventHandler * eh = dynamic_cast<EventHandler *>(_child);
    Handlers.Insert(Handlers.Count(),eh);
    return true;
}

//////////////IParserHandler Interfase///////////////////////

}}
