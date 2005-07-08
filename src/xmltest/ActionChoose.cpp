#include "ActionChoose.h"


ActionChoose::ActionChoose(const SectionParams& params)
{
    cout << "<choose> action created" << endl;
}

ActionChoose::~ActionChoose()
{
    int key;
    Action * value;

    for (IntHash<Action *>::Iterator it = WhenActions.First(); it.Next(key, value);)
    {
        delete value;
    }

    for (IntHash<Action *>::Iterator it = OtherwiseActions.First(); it.Next(key, value);)
    {
        delete value;
    }
    cout << "<choose> action released" << endl;
}

//////////////IParserHandler Interfase///////////////////////

void ActionChoose::StartXMLSubSection(const std::string& name,const SectionParams& params)
{
    if (name == "when") ActivateWhenSection = true;
    if (name == "otherwise") ActivateOtherwiseSection = true;
}

void ActionChoose::FinishXMLSubSection(const std::string& name)
{
    if (name == "when") ActivateWhenSection = false;
    if (name == "otherwise") ActivateOtherwiseSection = false;
}

bool ActionChoose::SetChildObject(const IParserHandler * child)
{
    if (!child) return false;

    IParserHandler * _child = const_cast<IParserHandler *> (child);
    Action * action = dynamic_cast<Action *>(_child);

    if (ActivateWhenSection) 
    {
        WhenActions.Insert(WhenActions.Count(),action);
        cout << "<choose>: child to <when> setted" << endl;
        return true;
    } else 
    if (ActivateOtherwiseSection) 
    {
        OtherwiseActions.Insert(OtherwiseActions.Count(),action);
        cout << "<choose>: child to <otherwise> setted" << endl;
        return true;
    }
    return false;
}

//////////////IParserHandler Interfase///////////////////////

