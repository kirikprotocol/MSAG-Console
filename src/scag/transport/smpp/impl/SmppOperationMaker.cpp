#include "SmppOperationMaker.h"
#include "scag/transport/smpp/base/SmppCommand2.h"
#include "scag/re/base/RuleEngine2.h"
#include "scag/sessions/base/Session2.h"
#include "scag/sessions/base/Operation.h"
#include "scag/exc/SCAGExceptions.h"
#include "core/synchronization/Mutex.hpp"
#include "SmppManager2.h"
#include "scag/util/HRTimer.h"
#include "scag/util/io/HexDump.h"

namespace scag2 {
namespace transport {
namespace smpp {

using namespace exceptions;
using namespace sessions;

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
currentTime_(time(0)),
log_(logger)
{
    if ( !cmd_.get() || !session_.get() ) {
        throw Exception("cmd or session is NULL");
    }
    SMS* sms = cmd_->get_sms();
    if ( !sms ) {
        smsc_log_debug(log_,"sms not found in command %p", cmd_.get());
    } else {
        SmppCommand::getSlicingParameters(*sms,sarmr_,currentIndex_,lastIndex_);
    }
}



void SmppOperationMaker::process( re::RuleStatus& st, scag2::re::actions::CommandProperty& cp, util::HRTiming* inhrt )
{
    util::HRTiming hrt(inhrt);
    const char* where;
    try {
        do {
            where = "setup";
            setupOperation( st, cp );
            hrt.mark("opmk.mkop");
            if ( st.status == re::STATUS_OK ) {
                smsc_log_debug(log_, "%s: RuleEngine processing..., continueExec=%u",
                               where_, session_->getLongCallContext().continueExec ? 1 : 0);
                where = "process";
                re::RuleEngine::Instance().process( *cmd_.get(), *session_.get(), st, cp, &hrt );
                hrt.mark("opmk.exec");
                smsc_log_debug(log_, "%s: RuleEngine processed: st.status=%d st.result=%d cmd.stat=%d",
                               where_, st.status, st.result, cmd_->get_status() );
            }

            where = "post";
            postProcess( st, cp );
            hrt.mark("opmk.post");

            if ( st.status == re::STATUS_LONG_CALL ) {
                smsc_log_debug( log_, "%s: long call initiate", where_ );
                where = "longcall";
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
        smsc_log_warn( log_, "%s: exception in opmaker: %s", where, e.what() );
        fail( e.what(), st, smsc::system::Status::SYSERR );
        
    } catch (...) {
        smsc_log_warn( log_, "%s: unknown exception in opmaker", where );
        fail( "unknown exception in opmaker", st, smsc::system::Status::SYSERR );

    }
}



void SmppOperationMaker::setupOperation( re::RuleStatus& st,
                                         re::actions::CommandProperty& cp )
{
    // find out operation type
    SMS* sms = cmd_->get_sms();
    if ( ! sms ) {
        fail( "SMS not found in command", st,
              smsc::system::Status::SYSERR );
        return;
    }

    Operation* op = 0;
    if ( cmd_->getOperationId() != invalidOpId() ) {

        // cmd has just returned from long call, or was rerouted
        op = session_->setCurrentOperation( cmd_->getOperationId() );
        if ( ! op ) {
            fail( "cmd->opid is set, but operation not found", st,
                  smsc::system::Status::TRANSACTIONTIMEDOUT );
            return;
        }
        const std::string* kw = op->getKeywords();
        if ( kw && !kw->empty() ) { cp.keywords = *kw; }

        // session_->getLongCallContext().continueExec = true;
        smsc_log_debug( log_, "cmd=%p continue op=%p opid=%d type=%d(%s) (no preprocess)",
                        cmd_.get(), op, cmd_->getOperationId(),
                        op->type(), commandOpName(op->type()) );
        st.status = re::STATUS_OK;
        st.result = 0;
        return;
    }

    const CommandId cmdid = CommandId(cmd_->getCommandId());

    /*
    int receiptMessageId = 0;
    if ( cmdid == DELIVERY || cmdid == DELIVERY_RESP ) {
        receiptMessageId = atoi(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
    }
     */

    optype_ = CO_NA;
    bool wantOpenUSSD = false;
    bool isUSSDClosed = false;

    const int ussd_op = 
        sms->hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP) ?
        int(sms->getIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP)) : -1;

    // fix for ussd_op == 35 is already applied in state machine

    switch ( cmdid )
    {
    case DELIVERY:

        /* if (receiptMessageId)
            optype_ = CO_RECEIPT;
        else */ 
        if ( ussd_op != -1 )
        {
            optype_ = CO_USSD_DIALOG;
            if ( ussd_op == smsc::sms::USSD_PSSR_IND ) {
                wantOpenUSSD = true;
            } else if ( ussd_op == smsc::sms::USSD_USSN_CONF ||
                        ussd_op == smsc::sms::USSD_USSR_CONF_LAST ||
                        ussd_op == smsc::sms::USSD_USSREL_IND ) {
                isUSSDClosed = true;
            } else if ( ussd_op != smsc::sms::USSD_USSR_CONF ) {
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
            if ( ussd_op == smsc::sms::USSD_USSR_REQ ||
                 ussd_op == smsc::sms::USSD_USSN_REQ ||
                 ussd_op == smsc::sms::USSD_USSR_REQ_LAST ||
                 ussd_op == smsc::sms::USSD_USSR_REQ_VLR ||
                 ussd_op == smsc::sms::USSD_USSN_REQ_VLR ||
                 ussd_op == smsc::sms::USSD_USSR_REQ_VLR_LAST ) {
                if ( ! sms->hasIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE) ) {
                    // umr is absent
                    cmd_->setFlag( SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG );
                    wantOpenUSSD = true;
                }
            } else if ( ussd_op == smsc::sms::USSD_PSSR_RESP ||
                        ussd_op == smsc::sms::USSD_USSREL_REQ ) {
                isUSSDClosed = true;
            } else if ( ussd_op == smsc::sms::USSD_USSN_REQ_VLR_LAST ||
                        ussd_op == smsc::sms::USSD_USSN_REQ_LAST ) {
                // commands that may open dialog (and close it unconditionally)
                if ( ! sms->hasIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE) ) {
                    // umr is absent
                    cmd_->setFlag( SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG );
                    wantOpenUSSD = true;
                }
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
        if ( opid == invalidOpId() ) {
            fail( "resp->orgCmd opid is not set", st,
                  smsc::system::Status::SYSERR );
            return;
        }
        op = session_->setCurrentOperation( opid, true );
        if ( ! op ) {
            fail( "resp->orgCmd opid is set, but operation not found", st,
                  smsc::system::Status::TRANSACTIONTIMEDOUT );
            return;
        }
        cmd_->setOperationId( opid );
        // reset keywords in resp
        if ( op->getKeywords() ) {
            op->setKeywords("");
        }
        smsc_log_debug( log_, "resp cmd=%p takes orig op=%p opid=%d type=%d(%s)",
                        cmd_.get(), op, opid, op->type(), commandOpName(op->type()) );
        break;
    }
    default:
        // unsupported command
        fail( "Unsupported command type", st, smsc::system::Status::SYSERR );
        return;
    } // switch on cmdid
    
    int32_t umr = Operation::invalidUSSDref();

    if ( ! op ) {
        if ( sarmr_ != 0 ) {
            // multipart sms
            const opid_type opid = cmd_->getEntity()->getSarMappingOpid(sarmr_,currentTime_);
            if ( opid != invalidOpId() ) {
                // restore from session
                op = session_->setCurrentOperation(opid,true);
                smsc_log_debug(log_,"multipart op %sfound: cmd=%p sess=%p/%s opid=%u type=%d(%s) sarmr/idx/tot=%d/%d/%d",
                               op ? "":"NOT ",
                               cmd_.get(), session_.get(), 
                               session_->sessionKey().toString().c_str(),
                               opid, optype_, commandOpName(optype_),
                               sarmr_, currentIndex_, lastIndex_ );
                if (op) {
                    if ( op->getSARref() != sarmr_ ) {
                        smsc_log_warn(log_,"multipart op sarmr=%d mismatch (in op:%d), old one will be replaced", sarmr_, op->getSARref() );
                        op = 0;
                    } else {
                        cmd_->setOperationId(opid);
                    }
                }
            }
        }
    }

    if ( ! op ) {

        if ( optype_ == CO_USSD_DIALOG ) {

            if ( sarmr_ != 0 ) {
                fail( "USSD: SAR fields found", st,
                      smsc::system::Status::USSDMSGTOOLONG );
                return;
            }

            if ( sms->hasIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE) ) {
                umr = sms->getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE);
            } else if ( ! wantOpenUSSD ) {
                fail( "USSD: no UMR is specified", st,
                      smsc::system::Status::USSDDLGREFMISM );
                return;
            }

            opid_type found_ussd = session_->getUSSDOperationId();

            if ( wantOpenUSSD ) {

                if ( found_ussd != invalidOpId() ) {
                    // ussd operation already exists
                    op = session_->setCurrentOperation(found_ussd);
                    if ( ! op ) {
                        fail("USSD: opid is set but op is not found", st,
                             smsc::system::Status::SYSERR );
                        return;
                    } else if ( time_t(op->getUSSDLastTime() + Session::ussdReplaceTimeout()) < currentTime_ ) {
                        // the operation is too old
                        smsc_log_info( log_, "current USSD dialog op=%p opid=%u is inactive during %u seconds and be replaced",
                                       op, found_ussd, unsigned(currentTime_ - op->getUSSDLastTime()) );
                        session_->closeCurrentOperation();
                    } else {
                        fail("USSD: active dialog exists", st,
                             smsc::system::Status::USSDBUSY );
                        return;
                    }
                }
                op = session_->createOperation( *cmd_.get(), optype_ );
                found_ussd = session_->getUSSDOperationId();
                if ( umr == Operation::invalidUSSDref() ) {
                    op->setFlag( OperationFlags::SERVICE_INITIATED_USSD_DIALOG );
                    umr = Operation::notsetUSSDref(); // ask to set umr in delivery
                }
                op->setUSSDref( umr );
                if ( cmdid == DELIVERY ) op->setFlag(OperationFlags::NEXTUSSDISSUBMIT);

                if ( isUSSDClosed ) {
                    op->setStatus( OPERATION_COMPLETED );
                }

            } else if ( found_ussd == invalidOpId() ) {
                // ussd operation not found
                fail( "USSD: dialog not found", st, 
                      smsc::system::Status::USSDDLGREFMISM );
                return;

            } else { // ussd op exists

                op = session_->setCurrentOperation( found_ussd, true );
                if ( ! op ) {
                    fail( "ussd opid is set, but operation not found", st,
                          smsc::system::Status::SYSERR );
                    return;
                }
                // reset keywords for the next operation
                if ( op->getKeywords() ) {
                    smsc_log_debug(log_,"resetting keywords for op=%p, opid=%u, umr=%d",
                                   op, found_ussd, umr );
                    op->setKeywords("");
                }
                cmd_->setOperationId( found_ussd );

                if ( umr != op->getUSSDref() ) {
                    // umr mismatch
                    if ( op->getUSSDref() == Operation::notsetUSSDref() &&
                         cmdid == DELIVERY &&
                         op->flagSet( OperationFlags::SERVICE_INITIATED_USSD_DIALOG ) ) {
                        // it was initiated by service
                        smsc_log_debug( log_, "service initiated dialog op=%p opid=%u got umr=%d",
                                        op, found_ussd, umr );
                        op->setUSSDref( umr );
                    } else {
                        fail( "USSD: umr mismatch", st,
                              smsc::system::Status::USSDDLGREFMISM );
                        return;
                    }
                } // if umr mismatch
                
                if ( op->flagSet(OperationFlags::NEXTUSSDISSUBMIT) == (cmdid == DELIVERY) ) {
                    // we wanted submit but received deliver, or vice versa
                    fail( "USSD: awaiting/received direction mismatch", st,
                          smsc::system::Status::USSDDLGREFMISM );
                    return;
                } // if direction mismatch

                if ( cmdid == DELIVERY ) op->setFlag(OperationFlags::NEXTUSSDISSUBMIT);
                else op->clearFlag(OperationFlags::NEXTUSSDISSUBMIT);

                // update ussd activity time
                op->setUSSDLastTime( currentTime_ );

                if ( isUSSDClosed )
                    op->setStatus( OPERATION_COMPLETED );
                else
                    op->setStatus( OPERATION_CONTINUED );

            } // if ussd op exists

        } else {
            // not USSD

            op = session_->createOperation( *cmd_.get(), optype_ );
            if ( sarmr_ != 0 ) {
                op->setSARref( sarmr_ );
                const opid_type opid = session_->getCurrentOperationId();
                if ( cmd_->getEntity()->setSarMappingOpid(sarmr_,opid) ) {
                    smsc_log_warn(log_,"multipart op sarmr/idx/tot=%x/%d/%d opid=%u replaced the old one on %s",
                                  sarmr_, currentIndex_, lastIndex_,
                                  opid, cmd_->getEntity()->getSystemId() );
                } else {
                    smsc_log_debug(log_,"multipart op mapping sarmr/idx/tot=%d/%d/%d opid=%u created on %s",
                                   sarmr_, currentIndex_, lastIndex_, opid, cmd_->getEntity()->getSystemId() );
                }
            }

        }

    } else {

        // restore optype_
        optype_ = CommandOperation(op->type());

    }

    // operation is created

    /*
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
     */

    // preprocess operation
    if ( cmd_->isResp() ) {
        switch (optype_) {
        case CO_DELIVER:
        case CO_SUBMIT:
        case CO_DATA_SC_2_SME:
        case CO_DATA_SME_2_SC:
        // case CO_RECEIPT:
            op->receiveNewResp( currentIndex_, lastIndex_ );
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
            op->receiveNewPart( currentIndex_, lastIndex_ );
            break;
        case CO_SUBMIT:
        case CO_DATA_SME_2_SC:
            op->receiveNewPart( currentIndex_, lastIndex_ );
            break;
            /*
        case CO_RECEIPT:
            op->receiveNewResp( currentIndex, lastIndex );
            break;
             */
        case CO_USSD_DIALOG:
            // none
            break;
        default:
            fail( "Unsupported operation", st, smsc::system::Status::SYSERR );
            return;
        }

    } // not resp

    smsc_log_debug( log_, "op=%p preprocd opid=%u type=%d(%s) umr=%d sarmr/idx/tot=%d/%d/%d stat=%d(%s) flags=%u parts/resps=%d/%d",
                    op, session_->getCurrentOperationId(),
                    op->type(), commandOpName(op->type()),
                    umr, sarmr_, currentIndex_, lastIndex_,
                    op->getStatus(), op->getNamedStatus(),
                    op->flags(), op->parts(), op->resps() );
    if ( sarmr_ != 0 &&
         op->getSARref() == sarmr_ &&
         op->getSARstatus().status == re::STATUS_FAILED ) {
        // we should fix multipart messages.
        // NOTE: resps should be fixed also to prevent their entering RE. 
        st = op->getSARstatus();
        smsc_log_debug(log_, "multipart has failed previously: res=%d", st.result );
    } else {
        st.status = re::STATUS_OK;
        st.result = 0;
    }
    return;
}


void SmppOperationMaker::postProcess( re::RuleStatus& st,
                                      re::actions::CommandProperty& cp )
{
    postproc_ = true;

    const char* what = "";
    const opid_type opid = cmd_->getOperationId();
    do { // fake loop

        if ( st.status == re::STATUS_LONG_CALL ) {
            what = "gone to longcall";
            break;
        }

        Operation* op = session_->getCurrentOperation();
        if ( ! op ) {
            if (st.status == re::STATUS_FAILED) {
                what = "failed already";
                break;
            }
            st.status = re::STATUS_FAILED;
            st.result = smsc::system::Status::SYSERR;
            what_ = "Logic error: no current operation is set";
            what = "no op is set";
            break;
        }

        {
            const std::string* kw = op->getKeywords();
            if ( kw && !kw->empty() ) cp.keywords = *kw;
        }

        if ( st.status == re::STATUS_FAILED ) {
            // generate a resp back to a submitter
            if ( !cmd_->isResp() ) {op->receiveNewResp(currentIndex_,lastIndex_);}
            if ( sarmr_ != 0 ) {
                // multipart
                if ( op->getStatus() == OPERATION_COMPLETED ) {
                    cmd_->getEntity()->delSarMapping(sarmr_);
                    session_->closeCurrentOperation();
                    what = "fail, multi closed";
                } else {
                    op->setSARstatus(st);
                    session_->setCurrentOperation(invalidOpId());
                    what = "fail, multi, not all parts";
                }
            } else {
                session_->closeCurrentOperation();
                what = "fail, closed";
            }
            break;
        }

        if ( op->type() == CO_USSD_DIALOG ) {

            if ( cmd_->isResp() ) {
                if ( op->getStatus() == OPERATION_COMPLETED ) {
                    what = "ussd completed, closed on resp";
                    session_->closeCurrentOperation();
                } else if ( cmd_->get_status() ) {
                    what = "ussd bad resp, closed";
                    session_->closeCurrentOperation();
                }
            }

        } else {

            // not long call, not fail
            if ( sarmr_ != 0 && st.status == re::STATUS_OK && !cmd_->isResp() ) {
                // check that command is directed to the same destination
                SmppEntity* dst = cmd_->getDstEntity();
                if ( !cmd_->getEntity()->checkSlicedDestination(sarmr_,dst,currentTime_) ) {
                    st.status = re::STATUS_FAILED;
                    st.result = smsc::system::Status::NOROUTE;
                    op->receiveNewResp(currentIndex_,lastIndex_);
                    if ( op->getStatus() == OPERATION_COMPLETED ) {
                        cmd_->getEntity()->delSarMapping(sarmr_);
                        session_->closeCurrentOperation();
                        what = "dst mismatch, fail, multi closed";
                    } else {
                        op->setSARstatus(st);
                        session_->setCurrentOperation(invalidOpId());
                        what = "dst mismatch, fail, not all parts";
                    }
                    break;
                }
            }

            // const bool waitreceipt = op->flagSet(OperationFlags::WAIT_RECEIPT);
            if ( op->getStatus() == OPERATION_COMPLETED ) {
                // all parts and resps received
                if ( sarmr_ != 0 ) {
                    cmd_->getEntity()->delSarMapping(sarmr_);
                    what = "multipart completed, closed";
                } else {
                    what = "completed, closed";
                }
                session_->closeCurrentOperation();
            }
            /*
            if ( waitreceipt ) {
                session_->createOperation( *cmd_.get(), CO_RECEIPT );
                // make sure receipt is not a current operation
                session_->setCurrentOperation( invalidOpId() );
            }
             */
        }

    } while ( false ); // fake loop
    smsc_log_debug(log_,"postproc: sess=%p/%s opid=%u %s",
                   session_.get(), session_->sessionKey().toString().c_str(),
                   opid, what);
}

} //smpp
} //transport
} //scag
