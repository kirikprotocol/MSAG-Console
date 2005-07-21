#include "ActionSet.h"
#include "scag/re/SAX2Print.hpp"

#include <iostream>

namespace scag { namespace re { namespace actions {

ActionSet::ActionSet()
{
}
        
ActionSet::~ActionSet()
{
    smsc_log_debug(logger,"Action 'set' released");
}

void ActionSet::init(const SectionParams& params)
{
    if ((!params.Exists("var"))|| (!params.Exists("value"))) throw Exception("Action 'set': missing 'var' and 'value' parameters");

    Variable = params["var"];
    Value = params["value"];

    FieldType ft;
    const char * name = 0;

    ft = ActionContext::Separate(Variable,name); 
    if (ft==ftUnknown) throw Exception("Action 'set': unrecognized variable prefix");
}


bool ActionSet::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'set'");
    Property * property = context.getProperty(Variable);
    if (!property) return true;

    property->setStr(Value);
    smsc_log_debug(logger,"Action 'set': property '"+Variable+"' set to '"+property->getStr()+"'");
    return true;
}


}}}
