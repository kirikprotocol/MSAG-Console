#include "ActionSet.h"
#include "scag/SAX2Print.hpp"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

ActionSet::ActionSet()
{
}
        
ActionSet::~ActionSet()
{
    smsc_log_debug(logger,"Action 'set' released");
}

void ActionSet::init(const SectionParams& params,PropertyObject propertyObject)
{
    if ((!params.Exists("var"))|| (!params.Exists("value"))) throw SCAGException("Action 'set': missing 'var' and 'value' parameters");

    Variable = params["var"];
    Value = params["value"];

    FieldType ft;
    const char * name = 0;

    ft = ActionContext::Separate(Variable,name); 
    if (ft==ftUnknown) 
        throw InvalidPropertyException("Action 'set': unrecognized variable prefix '%s' for 'var' parameter",Variable.c_str());

    AccessType at;
    std::string msg = "Action 'set': cannot set property '";

    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atWrite)) 
            throw InvalidPropertyException("Action 'set': cannot set property '%s' - no access to write",Variable.c_str());
    }

    ft = ActionContext::Separate(Value,name);
    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw InvalidPropertyException("Action 'set': cannot read property '%s' - no access",Value.c_str());
    }


    smsc_log_debug(logger,"Action 'set':: init");
}


bool ActionSet::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'set'");
    Property * property = context.getProperty(Variable);

    if (!property) 
    {
        smsc_log_warn(logger,std::string("Action 'set':: invalid property '")+Variable+std::string("'"));
        return true;
    }

    FieldType ft;
    const char * name = 0;
    ft = ActionContext::Separate(Value,name);
    if (ft == ftUnknown) 
    {
        property->setStr(Value);
    }
    else
    {
        Property * val = context.getProperty(Value);
        if (val) 
        {
            property->setStr(val->getStr());
        }
        else 
            smsc_log_warn(logger,"Action 'set': cannot initialize '"+Variable+"' with '"+Value+"' - no such property");
    }

    smsc_log_debug(logger,"Action 'set': property '"+Variable+"' set to '"+property->getStr()+"'");
    return true;
}

IParserHandler * ActionSet::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'return' cannot include child objects");
}

bool ActionSet::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}