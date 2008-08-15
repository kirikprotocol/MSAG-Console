#include "ActionRedirect2.h"
#include "scag/re/base/CommandAdapter2.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionRedirect::init(const SectionParams& params,PropertyObject propertyObject)
{
    if ((propertyObject.HandlerId != EH_SUBMIT_SM)&&(propertyObject.HandlerId != EH_DELIVER_SM)&&(propertyObject.HandlerId != EH_DATA_SM))
        throw SCAGException("Action 'redirect' Error. Details: Action can be used only in 'SUBMIT_SM', 'DELIVER_SM' or 'DATA_SM' handler.");

    m_fOAFieldType = CheckParameter(params, propertyObject, "redirect", "OA", false, true, strOA, m_bExistOA);
    m_fDAFieldType = CheckParameter(params, propertyObject, "redirect", "DA", false, true, strDA, m_bExistDA);

    if ((!m_bExistOA)&&(!m_bExistDA)) 
        throw SCAGException("Must exist at list one of 'OA' and 'DA' parameters");

    smsc_log_debug(logger,"Action 'redirect':: init");
}

bool ActionRedirect::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'redirect'");

    SmppCommandAdapter& smppAdapter = dynamic_cast<SmppCommandAdapter&>(*context.getCommand());

    if (smppAdapter.hasServiceOp()) 
    {
        smsc_log_warn(logger,"Action 'redirect' stopped. Details: Action cannot be used in USSD Dialog.");
        return true;
    }
    
    Address OA;
    Address DA;

    //////////////OA////////
    if (m_bExistOA) 
    {
        if (m_fOAFieldType == ftUnknown) 
        {
            Address tmp(strOA.c_str());
            OA = tmp;
        } else
        {
            Property * property = context.getProperty(strOA);

            if (!property) 
            {
                smsc_log_warn(logger,"Action 'redirect':: invalid property '%s'", strOA.c_str());
                return true;
            }
            Address tmp(property->getStr().c_str());
            OA = tmp;
        }
    }

    //////////////DA///////
    if (m_bExistDA) 
    {
        if (m_fDAFieldType == ftUnknown) 
        {
            Address tmp(strDA.c_str());
            DA = tmp;
        } else
        {
            Property * property = context.getProperty(strDA);
            if (!property) 
            {
                smsc_log_warn(logger,"Action 'redirect':: invalid property '%s'", strDA.c_str());
                return true;
            }
            Address tmp(property->getStr().c_str());
            DA = tmp;
        }
    }

    RuleStatus& rs = context.getRuleStatus();
    rs.status = STATUS_REDIRECT;

    if (m_bExistOA) smppAdapter.setOA(OA);
    if (m_bExistDA) smppAdapter.setDA(DA);

    context.clearLongCallContext();
    smsc_log_debug(logger,"Action 'redirect' finished");
    return false;
}

IParserHandler * ActionRedirect::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'redirect' cannot include child objects");
}

bool ActionRedirect::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}
