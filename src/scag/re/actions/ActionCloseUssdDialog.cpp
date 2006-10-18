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
    Property * propertyUSSD = context.getProperty("#ussd_dialog");
    Property * propertySetter = context.getProperty("#ussd_pssr_req");

    if ((!propertyUSSD)&&(!propertySetter))
    {
        smsc_log_warn(logger, "Action 'operation:close_ussd_dialog' stopped. Details: Cannot read 'ussd_dialog' property.");
        return true;
    }

    if (propertyUSSD->getBool()) propertySetter->setInt(PSSR_RESPONSE);

    return true;
}




}}}