#include "ActionCloseUssdDialog.h"
#include "scag/re/CommandAdapter.h"


namespace scag { namespace re { namespace actions {

IParserHandler * ActionCloseUssdDialog::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'operation:close_ussd_dialog': cannot have a child object");
}

bool ActionCloseUssdDialog::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionCloseUssdDialog::init(const SectionParams& params,PropertyObject propertyObject)
{
    if (propertyObject.HandlerId != EH_SUBMIT_SM) 
        throw SCAGException("Action 'operation:close_ussd_dialog' Error. Details: Action can be used only in 'SUBMIT_SM' handler.");
    
    logger = Logger::getInstance("scag.re");
}

bool ActionCloseUssdDialog::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'operation:close_ussd_dialog'");

    
    CommandAccessor& commandAccessor = context.getCommand();
    SmppCommandAdapter& smppAdapter = dynamic_cast<SmppCommandAdapter&>(commandAccessor);

    int serviceOp = 0;

    if (smppAdapter.hasServiceOp()) 
        serviceOp = smppAdapter.getServiceOp();
    else
    {
        smsc_log_warn(logger, "Action 'operation:close_ussd_dialog' stopped. Details: Command is not ussd-type.");
        return true;
    }

    if (serviceOp == USSR_REQUEST) 
    {
        smppAdapter.setServiceOp(PSSR_RESPONSE);
        smsc_log_debug(logger,"Action 'operation:close_ussd_dialog' closed ussd dialog.");
    }
    
    /*
    Property * propertyUSSD = context.getProperty("#ussd_dialog");

    if (!propertyUSSD) 
    {
        smsc_log_warn(logger, "Action 'operation:close_ussd_dialog' stopped. Details: Cannot read '#ussd_dialog' property.");
        return true;
    }

    Property * propertyUSSR_REQ = context.getProperty("#ussd_ussr_req");
    if (!propertyUSSR_REQ) 
    {
        smsc_log_warn(logger, "Action 'operation:close_ussd_dialog' stopped. Details: Cannot read '#ussd_ussr_req' property.");
        return true;
    }

    //if (propertyUSSD->getBool()&&propertyUSSR_REQ->getBool()) 
    {
        Property * propertySetter = context.getProperty("#ussd_pssr_resp");

        if (!propertySetter) 
        {
            smsc_log_warn(logger, "Action 'operation:close_ussd_dialog' stopped. Details: Cannot read '#ussd_pssr_resp' property.");
            return true;
        }

        propertySetter->setBool(true);
        smsc_log_debug(logger,"Action 'operation:close_ussd_dialog' closed ussd dialog.");
    }
    */
    return true;
}




}}}