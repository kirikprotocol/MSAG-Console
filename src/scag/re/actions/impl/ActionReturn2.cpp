#include "ActionReturn2.h"
#include "scag/re/base/CommandAdapter2.h"



namespace scag2 { namespace re { namespace actions {

ActionReturn::~ActionReturn()
{
    //smsc_log_debug(logger,"Action 'return' released");
}

void ActionReturn::init(const SectionParams& params,PropertyObject propertyObject)
{
    FieldType ft;
    std::string temp;
    bool bExist;
    std::string StatusStr;

    m_ftResult = CheckParameter(params, propertyObject, "return", "result", false, true, m_sResultValue, m_bResultExist);

    if ((m_bResultExist)&&(m_ftResult == ftUnknown)) m_nResultValue = atoi(m_sResultValue.c_str());

    FieldType ftStatus = CheckParameter(params, propertyObject, "return", "status", true, true, StatusStr, bExist);

    if (ftStatus != ftUnknown) 
        throw SCAGException("Action 'return': status must be 'Ok|Failed' type");

    if (StatusStr == "Ok") 
        m_bStatusValue = STATUS_OK;
    else if (StatusStr == "Failed") 
        m_bStatusValue = STATUS_FAILED;
    else
        throw SCAGException("Action 'return': status must be 'Ok|Failed' type");

    smsc_log_debug(logger,"Action 'return':: init...");
}


bool ActionReturn::run(ActionContext& context)
{
    RuleStatus& rs = context.getRuleStatus();

    if (m_bResultExist) 
    {
        if (m_ftResult == ftUnknown) 
            rs.result = m_nResultValue;
        else
        {
            Property * property = context.getProperty(m_sResultValue);
            if (property) 
                rs.result = int(property->getInt());
            else 
                smsc_log_warn(logger,"Action 'return': invalid property '%s' to return", m_sResultValue.c_str());
        }
    }

   rs.status = m_bStatusValue;


    smsc_log_debug(logger,"Action 'return': return result=%d, status=%d",rs.result, rs.status);

    context.clearLongCallContext();
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

