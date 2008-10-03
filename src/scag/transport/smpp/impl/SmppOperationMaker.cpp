#include "SmppOperationMaker.h"
#include "scag/transport/smpp/base/SmppCommand2.h"
#include "scag/re/base/RuleEngine2.h"
#include "scag/sessions/base/Session2.h"
#include "scag/sessions/base/Operation.h"
#include "scag/exc/SCAGExceptions.h"
#include "core/synchronization/Mutex.hpp"
#include "SmppManager2.h"

namespace scag2 {
namespace transport {
namespace smpp {

using namespace exceptions;

SmppOperationMaker::SmppOperationMaker( const char* where,
                                        std::auto_ptr<SmppCommand>& thecmd,
                                        sessions::ActiveSession&    thesession,
                                        smsc::logger::Logger*       logger ) :
where_(where),
cmd_(thecmd),
session_(thesession),
optype_(CO_NA),
what_(0),
postproc_(false),
log_(logger)
{
    assert( cmd_.get() && session_.get() );
}



void SmppOperationMaker::process( re::RuleStatus& st )
{
    try {
        do {

            setupOperation( st );
            if ( st.status != re::STATUS_OK ) break;
        
            smsc_log_debug(log_, "%s: RuleEngine processing...", where_ );            
            re::RuleEngine::Instance().process( *cmd_.get(), *session_.get(), st );
            smsc_log_debug(log_, "%s: RuleEngine processed: st.status=%d st.result=%d",
                           where_, st.status, st.result );
            postProcess( st );
            if ( st.status == re::STATUS_LONG_CALL ) {
                smsc_log_debug( log_, "%s: long call initiate", where_ );
                if ( SmppManager::Instance().makeLongCall(cmd_, session_) ) return;
                fail( "could not make long call", st, smsc::system::Status::SYSERR );
                return;
            } else if ( st.status == re::STATUS_OK || st.status == re::STATUS_REDIRECT ) {

                // ok

            } else {

                if ( ! st.result ) {
                    smsc_log_warn( log_, "%s: Rule failed and no error(zero result) returned",
                                   where_ );
                    st.result = smsc::system::Status::SYSERR;
                }
                
            }

        } while ( false );

    } catch ( std::exception& e ) {
        // smsc_log_warn( log_, "%s: exception in opmaker: %s", where, e.what() );
        fail( e.what(), st, smsc::system::Status::SYSERR );
        
    } catch (...) {
        // smsc_log_warn( log_, "%s: unknown exception in opmaker", where );
        fail( "unknown exception in opmaker", st, smsc::system::Status::SYSERR );

    }
}



void SmppOperationMaker::setupOperation( re::RuleStatus& st )
{
    // find out operation type
    SMS* sms = cmd_->get_sms();
    if ( ! sms ) {
        fail( "SMS not found in command", st,
              smsc::system::Status::SYSERR );
        return;
    }

    Operation* op = 0;
    if ( cmd_->getOperationId() != SCAGCommand::invalidOpId() ) {

        // cmd has just returned from long call, or was rerouted
        op = session_->setCurrentOperation( cmd_->getOperationId() );
        if ( ! op ) {
            fail( "cmd->opid is set, but operation not found", st,
                  smsc::system::Status::TRANSACTIONTIMEDOUT );
            return;
        }
        session_->getLongCallContext().continueExec = true;
        smsc_log_debug( log_, "cmd=%p continue op=%p opid=%d type=%d(%s) (no preprocess)",
                        cmd_.get(), op, cmd_->getOperationId(),
                        op->type(), commandOpName(op->type()) );
        st.status = re::STATUS_OK;
        st.result = 0;
        return;
    }

    const CommandId cmdid = CommandId(cmd_->getCommandId());

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

    // fix for ussd_op == 35 is already applied in state machine

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
                fail( "USSD Delivery: wrong ussd_op", st, 
                      smsc::system::Status::USSDDLGREFMISM );
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
                    cmd_->setFlag( SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG );
                    wantOpenUSSD = true;
                }
            } else if ( ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_RESPONSE ) {
                isUSSDClosed = true;
            } else {
                // invalid op
                fail( "USSD Submit: wrong ussd_op", st,
                      smsc::system::Status::USSDDLGREFMISM );
                return;
            }
        }
        else 
            optype_ = CO_SUBMIT;
        break;

    case DATASM:

        switch (cmd_->get_smsCommand().dir)
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
            fail( "cannot identify DATA_SM direction", st,
                  smsc::system::Status::SYSERR );
            return;
        }
        break;

    case DELIVERY_RESP:
    case SUBMIT_RESP :
    case DATASM_RESP : {
        // we simply take operation type from original command operation.
        // NOTE: as we have SMS we already have the original command,
        // so skip any checking here!
        opid_type opid = cmd_->get_resp()->getOrgCmd()->getOperationId();
        if ( opid == SCAGCommand::invalidOpId() ) {
            fail( "resp->orgCmd opid is not set", st,
                  smsc::system::Status::SYSERR );
            return;
        }
        op = session_->setCurrentOperation( opid );
        if ( ! op ) {
            fail( "resp->orgCmd opid is set, but operation not found", st,
                  smsc::system::Status::TRANSACTIONTIMEDOUT );
            return;
        }
        cmd_->setOperationId( opid );
        smsc_log_debug( log_, "resp cmd=%p takes orig op=%p opid=%d type=%d(%s)",
                        cmd_.get(), op, opid, op->type(), commandOpName(op->type()) );
        break;
    }
    default:
        // unsupported command
        fail( "Unsupported command type", st, smsc::system::Status::SYSERR );
        return;
    } // switch on cmdid
    
    int32_t umr = -1;

    // multipart sms
    // set slicing
    int lastIndex = 0;
    int currentIndex = 0;
    if ( sms->hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM) &&
         sms->hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS) &&
         sms->hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM) ) {

        // FIXME: restore the operation for multipart SMS
        lastIndex = sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        currentIndex = sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
    }

    if ( ! op ) {

        if ( optype_ == CO_USSD_DIALOG ) {

            if ( sms->hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ) {
                umr = sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
            } else if ( ! wantOpenUSSD ) {
                fail( "USSD: no UMR is specified", st,
                      smsc::system::Status::USSDDLGREFMISM );
                return;
            }
            
            opid_type found_ussd = session_->getUSSDOperationId();

            if ( wantOpenUSSD ) {

                if ( session_->getUSSDOperationId() != SCAGCommand::invalidOpId() ) {
                    op = session_->setCurrentOperation( found_ussd );
                    smsc_log_info( log_, "current USSD dialog op=%p opid=%u is replaced",
                                   op, found_ussd );
                    session_->closeCurrentOperation();
                }
                op = session_->createOperation( *cmd_.get(), optype_ );
                found_ussd = session_->getUSSDOperationId();
                if ( umr < 0 ) {
                    op->setFlag( OperationFlags::SERVICE_INITIATED_USSD_DIALOG );
                    umr = 0; // ask to set umr in delivery
                }
                session_->setUSSDref( umr );

            } else if ( found_ussd == SCAGCommand::invalidOpId() ) {
                // ussd operation not found
                fail( "USSD: dialog not found", st, 
                      smsc::system::Status::USSDDLGREFMISM );
                return;

            } else { // ussd op exists

                op = session_->setCurrentOperation( found_ussd );
                if ( ! op ) {
                    fail( "ussd opid is set, but operation not found", st,
                          smsc::system::Status::SYSERR );
                    return;
                }

                if ( umr != session_->getUSSDref() ) {
                    // umr mismatch
                    if ( session_->getUSSDref() == 0 && 
                         cmdid == DELIVERY &&
                         op->flagSet( OperationFlags::SERVICE_INITIATED_USSD_DIALOG ) ) {
                        // it was initiated by service
                        smsc_log_debug( log_, "service initiated dialog op=%p opid=%u got umr=%d",
                                        op, found_ussd, umr );
                        session_->setUSSDref( umr );
                    } else {
                        fail( "USSD: umr mismatch", st,
                              smsc::system::Status::USSDDLGREFMISM );
                        return;
                    }
                } // if umr mismatch
                
                if ( isUSSDClosed )
                    op->setStatus( OPERATION_COMPLETED );
                else
                    op->setStatus( OPERATION_CONTINUED );

            } // if ussd op exists

        } else {

            op = session_->createOperation( *cmd_.get(), optype_ );

        }

    } else {

        // restore optype_
        optype_ = CommandOperation(op->type());

    }

    // operation is created

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

    // preprocess operation
    if ( cmd_->isResp() ) {
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
            fail( "Unsupported resp operation", st, 
                  smsc::system::Status::SYSERR );
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
            fail( "Unsupported operation", st, smsc::system::Status::SYSERR );
            return;
        }

    } // not resp

    smsc_log_debug( log_, "op=%p preprocd opid=%u type=%d(%s) umr=%d stat=%d(%s) flags=%u parts=%d/%d",
                    op, session_->getCurrentOperationId(),
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
        session_->closeCurrentOperation();
        return;
    } else if ( st.status == re::STATUS_LONG_CALL ) {
        return;
    }

    Operation* op = session_->getCurrentOperation();
    if ( ! op ) {
        st.status = re::STATUS_FAILED;
        st.result = smsc::system::Status::SYSERR;
        what_ = "Logic error: no current operation is set";
        return;
    }

    if ( op->type() == CO_USSD_DIALOG ) {

        if ( op->getStatus() == OPERATION_COMPLETED && cmd_->isResp() ) {
            session_->closeCurrentOperation();
        }

    } else {

        if ( op->getStatus() == OPERATION_COMPLETED )
            session_->closeCurrentOperation();
        if ( op->flagSet(OperationFlags::WAIT_RECEIPT) ) {
            session_->createOperation( *cmd_.get(), CO_RECEIPT );
            // make sure receipt is not a current operation
            session_->setCurrentOperation( SCAGCommand::invalidOpId() );
        }

    }
}

} //smpp
} //transport
} //scag
