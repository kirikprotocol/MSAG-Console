#if 0

#include "SmppTransportRule.h"
#include "CommandBridge.h"
#include "scag/sessions/base/Operation.h"
#include "scag/sessions/base/Session2.h"

namespace scag2 {
namespace re {
namespace smpp {

void SmppTransportRule::ProcessModifyRespCommandOperation( Session& session,
                                                           SmppCommand& command,
                                                           CSmppDescriptor& smppDescriptor )
{
    Operation * operation = 0;

    if (session.getLongCallContext().continueExec )
    {
        operation = session.setCurrentOperation(command.getOperationId());
        smsc_log_debug( log_,"resp: continue execution: opid=%u, op=%p",
                        unsigned(command.getOperationId()), operation );
        return;
    }

    switch (smppDescriptor.cmdType)
    {
    case CO_DELIVER:
    case CO_SUBMIT:
    case CO_DATA_SC_2_SME:
    case CO_DATA_SME_2_SC:
    case CO_RECEIPT:
        operation = session.setCurrentOperation(command.getOperationId());
        operation->receiveNewResp(smppDescriptor.currentIndex,smppDescriptor.lastIndex);
        break;
    case CO_USSD_DIALOG:
        smsc_log_debug(log_,"Session: process USSD_DIALOG operation");
        operation = session.setCurrentOperation(session.getUSSDOperationId());

        if (smppDescriptor.isUSSDClosed) operation->setStatus(OPERATION_COMPLETED);
        else operation->setStatus(OPERATION_CONTINUED);

        break;
    default:
        // do nothing
        break;
    }
}


void SmppTransportRule::ProcessModifyCommandOperation( Session& session,
                                                       SmppCommand& command,
                                                       CSmppDescriptor& smppDescriptor )
{
    Operation * operation = 0;
    // uint16_t UMR;

    if ( session.getLongCallContext().continueExec ) {
        operation = session.setCurrentOperation(command.getOperationId());
        smsc_log_debug( log_, "continue execution. opid=%u, op=%p",
                        unsigned(command.getOperationId()),
                        operation );
        return;
    }

    switch (smppDescriptor.cmdType)
    {
    case CO_DELIVER:
    case CO_DATA_SC_2_SME:
        // if (session.hasPending()) 
        // operation = session.setOperationFromPending(command, smppDescriptor.cmdType);
        // else 
        if ( smppDescriptor.currentIndex <= 1 )
            // operation = session.AddNewOperationToHash(command, smppDescriptor.cmdType);
            operation = session.createOperation(command, smppDescriptor.cmdType);
        else
            operation = session.setCurrentOperation(command.getOperationId());
        operation->receiveNewPart(smppDescriptor.currentIndex,smppDescriptor.lastIndex);
        break;

    case CO_SUBMIT:
    case CO_DATA_SME_2_SC:
        //TODO: разобаться с RECEIPT`ом и SUBMIT_RESP

        // UMR = session.getUSR();

        /*
        if ((UMR <= 0)||(session.getCanOpenSubmitOperation()))
        {
            if (smppDescriptor.currentIndex == 0) 
                operation = session.AddNewOperationToHash(command, smppDescriptor.cmdType);
            else
                operation = session.setCurrentOperation(command.getOperationId());
        } 
        else if (smppDescriptor.currentIndex == 0)
            operation = session.setOperationFromPending(command, smppDescriptor.cmdType);
        else
            operation = session.setCurrentOperation(command.getOperationId());
         */
        if ( smppDescriptor.currentIndex == 0 )
            operation = session.createOperation( command, smppDescriptor.cmdType );
        else
            operation = session.setCurrentOperation(command.getOperationId());
        operation->receiveNewPart(smppDescriptor.currentIndex,smppDescriptor.lastIndex);
        break;

    case CO_RECEIPT:
        //TODO:: Нужно учесть политику для multipart

        operation = session.setCurrentOperation(command.getOperationId());
        operation->receiveNewResp(smppDescriptor.currentIndex,smppDescriptor.lastIndex);
        break;   

    case CO_USSD_DIALOG:
        smsc_log_debug(log_,"Session: process USSD_DIALOG operation");

        if (smppDescriptor.wantOpenUSSD)
        {
            // operation = session.AddNewOperationToHash(command, CO_USSD_DIALOG);
            operation = session.createOperation(command, CO_USSD_DIALOG);
            if ( command.flagSet(transport::smpp::SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG))
                operation->setFlag(sessions::OperationFlags::SERVICE_INITIATED_USSD_DIALOG);
            operation->setStatus(sessions::OPERATION_INITED);
            // set UMR
            command.get_sms()->setIntProperty( Tag::SMPP_USER_MESSAGE_REFERENCE,
                                               session.getUSSDOperationId() );
            break;
        }

        operation = session.setCurrentOperation(session.getUSSDOperationId());
        command.setOperationId(session.getCurrentOperationId());
        operation->setStatus(sessions::OPERATION_CONTINUED);
        break;

    default:
        break;
    }
}


void SmppTransportRule::ModifyOperationAfterExecuting( Session& session,
                                                       SmppCommand& command,
                                                       RuleStatus& ruleStatus,
                                                       CSmppDescriptor& smppDescriptor )
{
    Operation * currentOperation = session.getCurrentOperation();
    if ( !currentOperation ) return; // it may be already closed
    // throw SCAGException("Session: Fatal error - cannot end operation. Cause: current operation not found");

    if ( ruleStatus.status == re::STATUS_FAILED )
    {
        session.closeCurrentOperation(); 
        return;
    }

    if (ruleStatus.status == re::STATUS_LONG_CALL)
        return;

    smsc_log_debug( log_, "current operation=%p status=%d(%s)",
                    currentOperation, currentOperation->getStatus(),
                    currentOperation->getNamedStatus());

    switch ( smppDescriptor.cmdType )
    {
    case CO_DELIVER:
    case CO_DATA_SC_2_SME:
    case CO_RECEIPT:
        if (currentOperation->getStatus() == OPERATION_COMPLETED)
            session.closeCurrentOperation();
        break;

    case CO_SUBMIT:
    case CO_DATA_SME_2_SC:
        if (currentOperation->getStatus() == OPERATION_COMPLETED) 
        {
            session.closeCurrentOperation();
            break;
        }

        if (smppDescriptor.isResp) break; 

        if (smppDescriptor.m_waitReceipt)
        {
            /*time_t now;
            time(&now);

            PendingOperation pendingOperation;
            pendingOperation.type = CO_RECEIPT;
            //TODO: Определяем время жизни операции
            pendingOperation.validityTime = now + SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;


            session.addPendingOperation(pendingOperation);*/
            session.createOperation(command, CO_RECEIPT);
        }                    
        break;


    case CO_USSD_DIALOG:
        if ((smppDescriptor.isUSSDClosed) && (smppDescriptor.isResp))
            session.closeCurrentOperation();
        break;

    default :
        // do nothing
        break;
    }
}


void SmppTransportRule::setupSessionOperation( SCAGCommand& command, Session& session, RuleStatus& rs )
{
    SmppCommand& smppcommand = static_cast< SmppCommand& >( command );
    CSmppDescriptor smppDescriptor = CommandBridge::getSmppDescriptor(smppcommand);
    if ( smppDescriptor.isResp )
        ProcessModifyRespCommandOperation(session, smppcommand, smppDescriptor);
    else
        ProcessModifyCommandOperation(session, smppcommand, smppDescriptor);
}


void SmppTransportRule::resetSessionOperation( transport::SCAGCommand& command,
                                               sessions::Session&      session,
                                               RuleStatus&             rs )
{
    SmppCommand& smppcommand = static_cast< SmppCommand& >( command );
    CSmppDescriptor smppDescriptor = CommandBridge::getSmppDescriptor(smppcommand);
    ModifyOperationAfterExecuting( session, smppcommand, rs, smppDescriptor );
}

    
} // namespace smpp
} // namespace re
} // namespace scag2

#endif
