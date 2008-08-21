#include "SmppOperationMaker.h"
#include "SmppCommand2.h"
#include "scag/sessions/base/Session2.h"
#include "scag/sessions/base/Operation.h"
#include "scag/exc/SCAGExceptions.h"
#include "core/synchronization/Mutex.hpp"

namespace {
smsc::logger::Logger* log_ = 0;
smsc::core::synchronization::Mutex mtx;
}

namespace scag2 {
namespace transport {
namespace smpp {

using namespace exceptions;

SmppOperationMaker::SmppOperationMaker( SmppCommand& thecmd,
                                        sessions::Session& thesession ) :
cmd(thecmd),
session(thesession),
optype_(CO_NA),
what_(0),
postproc_(false) 
{
    if ( ! log_ ) {
        MutexGuard mg(mtx);
        if ( ! log_ ) log_ = smsc::logger::Logger::getInstance("smpp.opmk");
    }
}



void SmppOperationMaker::setupOperation( re::RuleStatus& st )
{
    // find out operation type
    SMS* sms = cmd.get_sms();
    if ( ! sms ) {
        what_ = "SMS not found in command";
        st.status = re::STATUS_FAILED;
        st.result = smsc::system::Status::SYSERR;
        return;
    }

    Operation* op = 0;
    if ( cmd.getOperationId() != SCAGCommand::invalidOpId() ) {

        // cmd has just returned from long call, or was rerouted
        op = session.setCurrentOperation( cmd.getOperationId() );
        if ( ! op ) {
            what_ = "cmd->opid is set, but operation not found";
            st.status = re::STATUS_FAILED;
            st.result = smsc::system::Status::SYSERR;
            return;
        }
        session.getLongCallContext().continueExec = true;
        smsc_log_debug( log_, "cmd=%p continue op=%p opid=%d type=%d(%s) (no preprocess)",
                        &cmd, op, cmd.getOperationId(),
                        op->type(), commandOpName(op->type()) );
        st.status = re::STATUS_OK;
        st.result = 0;
        return;
    }

    const CommandId cmdid = CommandId(cmd.getCommandId());

    int receiptMessageId = 0;
    if ( cmdid == DELIVERY || cmdid == DELIVERY_RESP ) {
        receiptMessageId = atoi(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
    }

    optype_ = CO_NA;
    bool wantOpenUSSD = false;
    bool isUSSDClosed = false;

    int8_t ussd_op = 
        sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
        int8_t(sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP)) : int8_t(-1);
    // FIXME: fix for ussd_op == 35

    switch ( cmdid )
    {
    case DELIVERY:

        if (receiptMessageId)
            optype_ = CO_RECEIPT;
        else if ( ussd_op != -1 )
        {
            optype_ = CO_USSD_DIALOG;
            if ( ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_INDICATION ) {
                wantOpenUSSD = true;
            } else if ( ussd_op == smsc::smpp::UssdServiceOpValue::USSN_CONFIRM ) {
                isUSSDClosed = true;
            } else if ( ussd_op != smsc::smpp::UssdServiceOpValue::USSR_CONFIRM ) {
                // wrong operation
                what_ = "USSD Delivery: wrong ussd_op";
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::USSDDLGREFMISM;
                return;
            }
        } else
            optype_ = CO_DELIVER;
        break;

    case SUBMIT:

        if (ussd_op != -1)
        {
            optype_ = CO_USSD_DIALOG;
            if ( ussd_op == smsc::smpp::UssdServiceOpValue::USSR_REQUEST ||
                 ussd_op == smsc::smpp::UssdServiceOpValue::USSN_REQUEST ) {
                if ( ! sms->hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ) {
                    // umr is absent
                    cmd.setFlag( SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG );
                    wantOpenUSSD = true;
                }
            } else if ( ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_RESPONSE ) {
                isUSSDClosed = true;
            } else {
                // invalid op
                what_ = "USSD Submit: wrong ussd_op";
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::USSDDLGREFMISM;
                return;
            }
        }
        else 
            optype_ = CO_SUBMIT;
        break;

    case DATASM:

        switch (cmd.get_smsCommand().dir)
        {
        case dsdSc2Sc:
            optype_ = CO_DATA_SC_2_SC;
            break;
        case dsdSc2Srv:
            optype_ = CO_DATA_SC_2_SME;
            break;
        case dsdSrv2Sc:
            optype_ = CO_DATA_SME_2_SC;
            break;
        case dsdSrv2Srv:
            optype_ = CO_DATA_SME_2_SME;
            break;
        case dsdUnknown:
        default:
            what_ = "cannot identify DATA_SM direction";
            st.status = re::STATUS_FAILED;
            st.result = smsc::system::Status::SYSERR;
            return;
        }
        break;

    case DELIVERY_RESP:
    case SUBMIT_RESP :
    case DATASM_RESP : {
        // we simply take operation type from original command operation.
        // NOTE: as we have SMS we already have the original command,
        // so skip any checking here!
        opid_type opid = cmd.get_resp()->getOrgCmd()->getOperationId();
        if ( opid == SCAGCommand::invalidOpId() ) {
            what_ = "resp->orgCmd opid is not set";
            st.status = re::STATUS_FAILED;
            st.result = smsc::system::Status::SYSERR;
            return;
        }
        op = session.setCurrentOperation( opid );
        if ( ! op ) {
            what_ = "resp->orgCmd opid is set, but operation not found";
            st.status = re::STATUS_FAILED;
            st.result = smsc::system::Status::SYSERR;
            return;
        }
        cmd.setOperationId( opid );
        smsc_log_debug( log_, "resp cmd=%p takes orig op=%p opid=%d type=%d(%s)",
                        &cmd, op, opid, op->type(), commandOpName(op->type()) );
        break;
    }
    default:
        // unsupported command
        what_ = "Unsupported command type";
        st.status = re::STATUS_FAILED;
        st.result = smsc::system::Status::SYSERR;
        return;
    } // switch on cmdid
    

    int32_t umr = -1;
    if ( ! op ) {

        if ( optype_ == CO_USSD_DIALOG ) {

            if ( sms->hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ) {
                umr = sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
            } else if ( ! wantOpenUSSD ) {
                what_ = "USSD: no UMR is specified";
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::USSDDLGREFMISM;
                return;
            }
            
            opid_type found_ussd = session.getUSSDOperationId();

            if ( wantOpenUSSD ) {

                if ( session.getUSSDOperationId() != SCAGCommand::invalidOpId() ) {
                    op = session.setCurrentOperation( found_ussd );
                    smsc_log_info( log_, "current USSD dialog op=%p opid=%u is replaced",
                                   op, found_ussd );
                    session.closeCurrentOperation();
                }
                op = session.createOperation( cmd, optype_ );
                found_ussd = session.getUSSDOperationId();
                if ( umr < 0 ) {
                    op->setFlag( OperationFlags::SERVICE_INITIATED_USSD_DIALOG );
                    umr = 0; // ask to set umr in delivery
                }
                session.setUSSDref( umr );

            } else if ( found_ussd == SCAGCommand::invalidOpId() ) {
                // ussd operation not found
                what_ = "USSD: dialog not found";
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::USSDDLGREFMISM;
                return;

            } else { // ussd op exists

                op = session.setCurrentOperation( found_ussd );
                if ( ! op ) {
                    what_ = "ussd opid is set, but operation not found";
                    st.status = re::STATUS_FAILED;
                    st.result = smsc::system::Status::SYSERR;
                    return;
                }

                if ( umr != session.getUSSDref() ) {
                    // umr mismatch
                    if ( session.getUSSDref() == 0 && 
                         cmdid == DELIVERY &&
                         op->flagSet( OperationFlags::SERVICE_INITIATED_USSD_DIALOG ) ) {
                        // it was initiated by service
                        smsc_log_debug( log_, "service initiated dialog op=%p opid=%u got umr=%d",
                                        op, found_ussd, umr );
                        session.setUSSDref( umr );
                    } else {
                        what_ = "USSD: umr mismatch";
                        st.status = re::STATUS_FAILED;
                        st.result = smsc::system::Status::USSDDLGREFMISM;
                        return;
                    }
                } // if umr mismatch
                
                if ( isUSSDClosed )
                    op->setStatus( OPERATION_COMPLETED );
                else
                    op->setStatus( OPERATION_CONTINUED );

            } // if ussd op exists

        } else {

            op = session.createOperation( cmd, optype_ );

        }

    } else {

        // restore optype_
        optype_ = CommandOperation(op->type());

    }

    { // setting waitReceipt
        bool transact = false;
        bool req_receipt = false;

        if (sms->hasIntProperty(Tag::SMPP_ESM_CLASS))
        {
            //узнаём транзакционная или нет доставка
            int esm_class = sms->getIntProperty(Tag::SMPP_ESM_CLASS);
            transact = ((esm_class&2) == 2);
        }

        if (sms->hasIntProperty(Tag::SMPP_REGISTRED_DELIVERY))
        {
            //узнаём заказал ли сервис отчёт о доставке
            int reg_delivery = sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY);
            req_receipt = ((reg_delivery&3) > 0);
        }
        if ( (!transact) && (req_receipt) ) {
            op->setFlag( OperationFlags::WAIT_RECEIPT );
        }
    }

    // set slicing
    int lastIndex = 0;
    if (sms->hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS)) 
        lastIndex = sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);

    int currentIndex = 0;
    if (sms->hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)) 
        currentIndex = sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

    // preprocess operation
    if ( cmd.isResp() ) {
        switch (optype_) {
        case CO_DELIVER:
        case CO_SUBMIT:
        case CO_DATA_SC_2_SME:
        case CO_DATA_SME_2_SC:
        case CO_RECEIPT:
            op->receiveNewResp( currentIndex, lastIndex );
            break;
        case CO_USSD_DIALOG:
            break;
        default:
            what_ = "Unsupported resp operation";
            st.status = re::STATUS_FAILED;
            st.result = smsc::system::Status::SYSERR;
            return;
        }

    } else { // not resp

        switch (optype_) {

        case CO_DELIVER:
        case CO_DATA_SC_2_SME:
            op->receiveNewPart( currentIndex, lastIndex );
            break;
        case CO_SUBMIT:
        case CO_DATA_SME_2_SC:
            op->receiveNewPart( currentIndex, lastIndex );
            break;
        case CO_RECEIPT:
            op->receiveNewResp( currentIndex, lastIndex );
            break;
        case CO_USSD_DIALOG:
            // none
            break;
        default:
            what_ = "Unsupported operation";
            st.status = re::STATUS_FAILED;
            st.result = smsc::system::Status::SYSERR;
            return;
        }

    } // not resp

    smsc_log_debug( log_, "op=%p preprocd opid=%u type=%d(%s) umr=%d stat=%d(%s) flags=%u parts=%d/%d",
                    op, session.getCurrentOperationId(),
                    op->type(), commandOpName(op->type()),
                    umr,
                    op->getStatus(), op->getNamedStatus(),
                    op->flags(), op->parts(), op->resps() );

    st.status = re::STATUS_OK;
    st.result = 0;
    return;
}


void SmppOperationMaker::postProcess( re::RuleStatus& st )
{
    postproc_ = true;

    if ( st.status == re::STATUS_FAILED ) {
        session.closeCurrentOperation();
        return;
    } else if ( st.status == re::STATUS_LONG_CALL ) {
        return;
    }

    Operation* op = session.getCurrentOperation();
    if ( ! op ) {
        st.status = re::STATUS_FAILED;
        st.result = smsc::system::Status::SYSERR;
        what_ = "Logic error: no current operation is set";
        return;
    }

    if ( op->type() == CO_USSD_DIALOG ) {

        if ( op->getStatus() == OPERATION_COMPLETED && cmd.isResp() ) {
            session.closeCurrentOperation();
        }

    } else {

        if ( op->getStatus() == OPERATION_COMPLETED )
            session.closeCurrentOperation();
        if ( op->flagSet(OperationFlags::WAIT_RECEIPT) ) {
            session.createOperation( cmd, CO_RECEIPT );
            // make sure receipt is not a current operation
            session.setCurrentOperation( SCAGCommand::invalidOpId() );
        }

    }
}


} //smpp
} //transport
} //scag
