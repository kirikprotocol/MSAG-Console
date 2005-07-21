#include "ActionReturn.h"
#include "scag/re/SAX2Print.hpp"

#include <iostream>

namespace scag { namespace re { namespace actions {

ActionReturn::~ActionReturn()
{
    smsc_log_debug(logger,"Action 'return' released");
}

void ActionReturn::init(const SectionParams& params)
{
    if (params.Exists("result")) 
    {
        ReturnValue = params["result"];
        if (ReturnValue.empty()) throw Exception("Action 'return': missing variable to return");
    }
    else 
        throw Exception("Action 'return': missing 'result' parameter");
   
}

bool ActionReturn::run(ActionContext& context)
{
    FieldType ft;

    const char * name = 0;
    ft = ActionContext::Separate(ReturnValue,name);
    RuleStatus rs;

    if (ft==ftUnknown) 
    {
        rs.result = atoi(ReturnValue.c_str());
    }
    else
    {
        Property * property = context.getProperty(ReturnValue);
        if (property) rs.result = property->getBool();
        else smsc_log_warn(logger,"Action 'return': invalid property '" + ReturnValue + "' to return ");
    }
    context.SetRuleStatus(rs);
    return false;
}


}}}

