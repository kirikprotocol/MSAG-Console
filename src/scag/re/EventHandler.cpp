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
    cout << "EventHandler released" << endl;
}

bool EventHandler::SetChildObject(const IParserHandler * child)
{
    if (!child) return false;

    IParserHandler * _child = const_cast<IParserHandler *> (child);
    Action * action = dynamic_cast<Action *>(_child);

    cout << "<handler>: child setted" << endl;
    actions.Insert(actions.Count(),action);
    return true;
}


}}
