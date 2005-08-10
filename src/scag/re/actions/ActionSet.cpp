#include "ActionSet.h"
#include "scag/re/SAX2Print.hpp"
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
    if ((!params.Exists("var"))|| (!params.Exists("value"))) throw Exception("Action 'set': missing 'var' and 'value' parameters");

    Variable = params["var"];
    Value = params["value"];

    FieldType ft;
    const char * name = 0;

    ft = ActionContext::Separate(Variable,name); 
    if (ft==ftUnknown) throw Exception("Action 'set': unrecognized variable prefix");

    AccessType at;
    std::string msg = "Action 'set': cannot set property '";

    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atWrite)) 
        {
            msg.append(Variable);
            msg.append("' - no access to write");
            throw Exception(msg.c_str());
        }
    }

    ft = ActionContext::Separate(Value,name);
    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
        {
            msg.append(Value);
            msg.append("' - no access to read");
            throw Exception(msg.c_str());
        }
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
    throw Exception("Action 'return' cannot include child objects");
}

bool ActionSet::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}