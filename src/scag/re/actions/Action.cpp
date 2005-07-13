#include "Action.h"

#include <iostream>

namespace scag { namespace re { namespace actions {

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  ACTION RETURN  /////////////////////////////////////
ActionReturn::ActionReturn(const SectionParams& params)
{
    cout << "<return> action created" << endl;
}

ActionReturn::~ActionReturn()
{
    cout << "<return> action released" << endl;
    
}


///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  ACTION SET  ////////////////////////////////////////

ActionSet::ActionSet(const SectionParams& params)
{
    Variable = "";
    Value = "";
    if ((!params.Exists("var"))|| (!params.Exists("value"))) return;

    Variable = params["var"];
    Value = params["value"];

    cout << "<set> action created " << Variable << "=" << Value << endl;
}
        
ActionSet::~ActionSet()
{
    cout << "<set> action released" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  ACTION IF  /////////////////////////////////////////

ActionIf::ActionIf(const SectionParams& params)
{
    ActivateThenSection = false;
    ActivateThenSection = false;

    Variable = "";
    Operation = "";
    Value = "";

    Variable = params["test"];

    if ((params.Exists("op"))&&(params.Exists("value"))) 
    {
        Operation = params["op"];
        Value = params["value"];
    }


    cout << "<if> action created " << Variable << " " << Operation << " " << Value << endl;
}

ActionIf::~ActionIf()
{
    int key;
    Action * value;

    for (IntHash<Action *>::Iterator it = ThenActions.First(); it.Next(key, value);)
    {
        delete value;
    }

    for (IntHash<Action *>::Iterator it = ElseActions.First(); it.Next(key, value);)
    {
        delete value;
    }
    cout << "<if> action released" << endl;
}


void ActionIf::StartXMLSubSection(const std::string& name,const SectionParams& params)
{
    if (name == "then") ActivateThenSection = true;
    if (name == "else") ActivateElseSection = true;
}

void ActionIf::FinishXMLSubSection(const std::string& name)
{
    if (name == "then") ActivateThenSection = false;
    if (name == "else") ActivateElseSection = false;
}

bool ActionIf::SetChildObject(const IParserHandler * child)
{
    if (!child) return false;


    IParserHandler * _child = const_cast<IParserHandler *> (child);
    Action * action = dynamic_cast<Action *>(_child);

    if (ActivateThenSection) 
    {
        ThenActions.Insert(ThenActions.Count(),action);
        cout << "<if>: child to <then> setted" << endl;
        return true;
    } else 
    if (ActivateElseSection) 
    {
        ElseActions.Insert(ThenActions.Count(),action);
        cout << "<if>: child to <else> setted" << endl;
        return true;
    }
    return false;
}

}}}
