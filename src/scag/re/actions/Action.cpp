#include "Action.h"

#include <iostream>

namespace scag { namespace re { namespace actions {

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  ACTION RETURN  /////////////////////////////////////
ActionReturn::ActionReturn(const SectionParams& params)
{
    ReturnValue = "";
    if (params.Exists("result")) ReturnValue = params["result"];

    cout << "<return> action created " << ReturnValue << endl;
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
    FillThenSection = false;
    FillThenSection = false;

    if params.Exists("test") singleparam.Variable = params["test"];

    if ((params.Exists("op"))&&(params.Exists("value"))) 
    {
        singleparam.Operation = params["op"];
        singleparam.Value = params["value"];
    }


    cout << "<if> action created " << singleparam.Variable << " " << singleparam.Operation << " " << singleparam.Value << endl;
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
    if (name == "then") FillThenSection = true;
    if (name == "else") FillElseSection = true;
}

void ActionIf::FinishXMLSubSection(const std::string& name)
{
    if (name == "then") FillThenSection = false;
    if (name == "else") FillElseSection = false;
}

bool ActionIf::SetChildObject(const IParserHandler * child)
{
    if (!child) return false;


    IParserHandler * _child = const_cast<IParserHandler *> (child);
    Action * action = dynamic_cast<Action *>(_child);

    if (!action) return false;

    if (FillThenSection) 
    {
        ThenActions.Insert(ThenActions.Count(),action);
        cout << "<if>: child to <then> setted" << endl;
        return true;
    } else 
    if (FillElseSection) 
    {
        ElseActions.Insert(ThenActions.Count(),action);
        cout << "<if>: child to <else> setted" << endl;
        return true;
    }
    return false;
}

}}}
