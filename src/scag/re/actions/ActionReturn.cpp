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

    m_ftResult = CheckParameter(params, propertyObject, "return", "result", false, true, temp, m_bResultExist);

    if (m_bResultExist) 
    {
        m_sResultValue = ConvertWStrToStr(temp);

        if (m_ftResult == ftUnknown) 
            m_nResultValue = atoi(m_sResultValue.c_str());
    }

    m_ftStatus = CheckParameter(params, propertyObject, "return", "status", true, true, temp, bExist);
    m_sStatusValue = ConvertWStrToStr(temp);

    if (m_ftStatus == ftUnknown) 
        m_bStatusValue = atoi(m_sStatusValue.c_str());


    smsc_log_debug(logger,"Action 'return':: init...");
}


bool ActionReturn::run(ActionContext& context)
{
    RuleStatus rs = context.getRuleStatus();

    if (m_bResultExist) 
    {
        if (m_ftResult == ftUnknown) 
            rs.result = m_nResultValue;
        else
        {
            Property * property = context.getProperty(m_sResultValue);
            if (property) 
                rs.result = property->getInt();
            else 
                smsc_log_warn(logger,"Action 'return': invalid property '%s' to return", m_sResultValue.c_str());
        }
    }

    if (m_ftStatus == ftUnknown) 
        rs.status = m_bStatusValue;
    else
    {
        Property * property = context.getProperty(m_sStatusValue);
        if (property) 
            rs.status = property->getBool();
        else 
            smsc_log_warn(logger,"Action 'return': invalid property '%s' to return", m_sStatusValue.c_str());
    }


    context.setRuleStatus(rs);
    smsc_log_debug(logger,"Action 'return': return result=%d, status=%d",rs.result, rs.status);

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

