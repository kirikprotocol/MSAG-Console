#include "ActionReturn.h"
#include "scag/re/CommandAdapter.h"



namespace scag { namespace re { namespace actions {

ActionReturn::~ActionReturn()
{
    //smsc_log_debug(logger,"Action 'return' released");
}

void ActionReturn::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    FieldType ft;
    std::string temp;
    bool bExist;

    m_ftResult = CheckParameter(params, propertyObject, "return", "result", true, true, temp, bExist);
    ReturnValue = ConvertWStrToStr(temp);

    smsc_log_debug(logger,"Action 'return':: init...");
}


bool ActionReturn::run(ActionContext& context)
{
    RuleStatus rs = context.getRuleStatus();

    if (m_ftResult==ftUnknown) 
        rs.result = atoi(ReturnValue.c_str());
    else
    {
        Property * property = context.getProperty(ReturnValue);
        if (property) 
            rs.result = property->getBool();
        else 
            smsc_log_warn(logger,"Action 'return': invalid property '%s' to return", ReturnValue.c_str());
    }

    context.setRuleStatus(rs);
    smsc_log_debug(logger,"Action 'return': return result=%d",rs.result);

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

