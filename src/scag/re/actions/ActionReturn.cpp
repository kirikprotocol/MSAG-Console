#include "ActionReturn.h"
#include "scag/re/CommandAdapter.h"

#include "scag/re/SAX2Print.hpp"


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
        if (ReturnValue.empty()) throw Exception("Action 'return': missing variable to return");
    }
    else 
        throw Exception("Action 'return': missing 'result' parameter");

    FieldType ft;

    const char * name = 0;
    ft = ActionContext::Separate(ReturnValue,name); 
    if (ft == ftField) 
    {
        AccessType at;
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
        {
            std::string msg = "Action 'return': cannot read property '";
            msg.append(ReturnValue);
            msg.append("' - no acces");
            throw Exception(msg.c_str());
        }
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
        else smsc_log_warn(logger,"Action 'return': invalid property '" + ReturnValue + "' to return ");
    }
    context.SetRuleStatus(rs);
    return false;
}


IParserHandler * ActionReturn::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw Exception("Action 'return' cannot include child objects");
}

bool ActionReturn::FinishXMLSubSection(const std::string& name)
{
    return true;
}


}}}

