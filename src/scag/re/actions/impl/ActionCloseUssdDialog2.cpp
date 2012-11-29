#include "ActionCloseUssdDialog2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/sessions/base/Operation.h"
#include "scag/sessions/base/Session2.h"
#include "sms/sms_const.h"

namespace scag2 {
namespace re {
namespace actions {

IParserHandler * ActionCloseUssdDialog::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'smpp:close_ussd_dialog': cannot have a child object");
}

bool ActionCloseUssdDialog::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionCloseUssdDialog::init(const SectionParams& params,PropertyObject propertyObject)
{
    if (propertyObject.HandlerId != EH_SUBMIT_SM) 
        throw SCAGException("Action 'smpp:close_ussd_dialog' Error. Details: Action can be used only in 'SUBMIT_SM' handler.");
}

bool ActionCloseUssdDialog::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'smpp:close_ussd_dialog'");

    SmppCommandAdapter& smppAdapter = dynamic_cast<SmppCommandAdapter&>(*context.getCommand());

    if ( !smppAdapter.hasServiceOp()) {
        smsc_log_warn(logger, "Action 'smpp:close_ussd_dialog' stopped. Details: Command is not ussd-type.");
        return true;
    }

    const int serviceOp = smppAdapter.getServiceOp();
    if ( ! ( serviceOp == smsc::sms::USSD_PSSR_IND ||
             serviceOp == smsc::sms::USSD_USSR_CONF ||
             serviceOp == smsc::sms::USSD_USSN_CONF ||
             serviceOp == smsc::sms::USSD_USSR_REQ ||
             serviceOp == smsc::sms::USSD_USSR_REQ_VLR ||
             serviceOp == smsc::sms::USSD_USSN_REQ_VLR
           ) ) {
        // will be closed automatically
        return true;
    }

    Operation* op = context.getSession().getCurrentOperation();
    const bool netinit = 
        ( op && op->flagSet(sessions::OperationFlags::SERVICE_INITIATED_USSD_DIALOG) );

    bool smppplus = false;
    SMS& sms = smppAdapter.getSMS();
    if ( sms.hasIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE) ) {
        const int32_t umr = sms.getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE);
        if ( (umr & 0x80000000) ) { smppplus = true; }
    }


    if ( !smppplus ) {
        // old protocol

        // USSD_PSSD_IND      dlv  open clowait
        // USSD_PSSR_IND      dlv  open
        // USSD_USSR_CONF     dlv
        // USSD_USSN_CONF     dlv  close
        // USSD_USSR_REQ      sbm  (open)
        // USSD_USSN_REQ      sbm  (open) clowait
        // USSD_PSSD_RESP     sbm  close
        // USSD_PSSR_RESP     sbm  close

        if ( serviceOp == smsc::sms::USSD_PSSR_IND ) {
            // opening dialog.
            // FIXME: The best thing to do is to replace with USSD_PSSD_IND
            // but unfortunately the SMSC by skv does not support it yet.
            // So do nothing.
        } else if ( serviceOp == smsc::sms::USSD_USSR_REQ ) {
            if (netinit) {
                smppAdapter.setServiceOp(smsc::sms::USSD_USSN_REQ);
            } else {
                // mobile init (close immediate)
                smppAdapter.setServiceOp(smsc::sms::USSD_PSSR_RESP);
                if (op) op->setStatus( OPERATION_COMPLETED );
            }
        } else if ( serviceOp == smsc::sms::USSD_USSR_CONF ) {
            if (netinit) {
                smppAdapter.setServiceOp(smsc::sms::USSD_USSN_CONF);
            } else {
                smppAdapter.setServiceOp(smsc::sms::USSD_PSSR_RESP);
            }
            if (op) op->setStatus( OPERATION_COMPLETED );
        }
        return true;
    }

    // smpp+

    // USSD_USSR_REQ            sbm  (open)
    // USSD_USSN_REQ            sbm  (open) ?clowait?
    // USSD_PSSD_RESP           sbm  close
    // USSD_PSSR_RESP           sbm  close
    // USSD_PSSD_IND            dlv  open clowait
    // USSD_PSSR_IND            dlv  open
    // USSD_USSR_CONF           dlv
    // USSD_USSN_CONF           dlv

    // USSD_USSREL_IND          dlv  close
    // USSD_USSR_CONF_LAST      dlv  close
    // USSD_USSREL_REQ          sbm  close
    // USSD_USSR_REQ_LAST       sbm  (open) clowait
    // USSD_USSN_REQ_LAST       sbm  (open) close
    // USSD_USSR_REQ_VLR        sbm  (open)
    // USSD_USSN_REQ_VLR        sbm  (open)
    // USSD_USSR_REQ_VLR_LAST   sbm  (open) clowait
    // USSD_USSN_REQ_VLR_LAST   sbm  (open) close

    if ( serviceOp == smsc::sms::USSD_PSSR_IND ) {
        // FIXME see comment above
    } else if ( serviceOp == smsc::sms::USSD_USSR_CONF ) {
        if (netinit) {
            smppAdapter.setServiceOp(smsc::sms::USSD_USSR_CONF_LAST);
        } else {
            smppAdapter.setServiceOp(smsc::sms::USSD_USSREL_IND);
        }
        if (op) op->setStatus(OPERATION_COMPLETED);
    } else if ( serviceOp == smsc::sms::USSD_USSN_CONF ) {
        if (netinit) {
            smppAdapter.setServiceOp(smsc::sms::USSD_USSR_CONF_LAST);
        } else {
            smppAdapter.setServiceOp(smsc::sms::USSD_USSREL_IND);
        }
        if (op) op->setStatus(OPERATION_COMPLETED);
    } else if ( serviceOp == smsc::sms::USSD_USSR_REQ ) {
        if (netinit) {
            smppAdapter.setServiceOp(smsc::sms::USSD_USSR_REQ_LAST);
        } else {
            // mobile init (close immediate)
            smppAdapter.setServiceOp(smsc::sms::USSD_PSSR_RESP);
            if (op) op->setStatus( OPERATION_COMPLETED );
        }
    } else if ( serviceOp == smsc::sms::USSD_USSR_REQ_VLR ) {
        if (netinit) {
            smppAdapter.setServiceOp(smsc::sms::USSD_USSR_REQ_VLR_LAST);
        } else {
            // mobile init (close immediate)
            smppAdapter.setServiceOp(smsc::sms::USSD_PSSR_RESP);
            if (op) op->setStatus( OPERATION_COMPLETED );
        }
    } else if ( serviceOp == smsc::sms::USSD_USSN_REQ_VLR ) {
        if (netinit) {
            smppAdapter.setServiceOp(smsc::sms::USSD_USSN_REQ_VLR_LAST);
        } else {
            // mobile init (close immediate)
            smppAdapter.setServiceOp(smsc::sms::USSD_PSSR_RESP);
        }
        if (op) op->setStatus( OPERATION_COMPLETED );
    }
    smsc_log_debug(logger,"Action 'smpp:close_ussd_dialog' closed ussd dialog.");
    return true;
}

}}}
