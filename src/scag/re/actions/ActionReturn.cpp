#include "ActionReturn.h"
#include "scag/re/CommandAdapter.h"

#include "scag/SAX2Print.hpp"


namespace scag { namespace re { namespace actions {

ActionReturn::~ActionReturn()
{
    smsc_log_debug(logger,"Action 'return' released");
}

void ActionReturn::init(const SectionParams& params,PropertyObject propertyObject)
{
    if (params.Exists("result")) 
    {
        ReturnValue = params["result"];
        if (ReturnValue.empty()) throw SCAGException("Action 'return': missing variable to return");
    }
    else 
        throw SCAGException("Action 'return': missing 'result' parameter");

    FieldType ft;

    const char * name = 0;
    ft = ActionContext::Separate(ReturnValue,name); 
    if (ft == ftField) 
    {
        AccessType at;
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw InvalidPropertyException("Action 'return': cannot read property '%s' - no acces",ReturnValue.c_str());
    }

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
        else smsc_log_warn(logger,"Action 'return': invalid property '%s' to return", ReturnValue.c_str());
    }
    context.SetRuleStatus(rs);
    return false;
}


IParserHandler * ActionReturn::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'return' cannot include child objects");
}

bool ActionReturn::FinishXMLSubSection(const std::string& name)
{
    return true;
}


}}}

