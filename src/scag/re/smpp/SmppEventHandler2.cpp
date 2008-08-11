#include "SmppEventHandler2.h"
#include "scag/re/smpp/SmppAdapter2.h"
#include "scag/re/actions/ActionContext2.h"
#include "scag/re/CommandBridge.h"
#include "scag/re/RuleEngine2.h"
#include "scag/sessions/Operation.h"

namespace scag2 {
namespace re {

using namespace smpp;

#if 0
void SmppEventHandler::ProcessModifyRespCommandOperation(Session& session, SmppCommand& command, CSmppDescriptor& smppDescriptor)
{
    Operation * operation = 0;

    if(session.getLongCallContext().continueExec)
    {
        smsc_log_debug(logger,"resp: continue execution. operation id=%u", unsigned(command.getOperationId()));
//        operation = session.setCurrentOperation(command.getOperationId());
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
        smsc_log_debug(logger,"Session: process USSD_DIALOG operation");
        operation = session.setCurrentOperation(session.getUSSDOperationId());

        if (smppDescriptor.isUSSDClosed) operation->setStatus(OPERATION_COMPLETED);
        else operation->setStatus(OPERATION_CONTINUED);

        break;
    default:
        // do nothing
        break;
    }
}

void SmppEventHandler::ProcessModifyCommandOperation(Session& session, SmppCommand& command, CSmppDescriptor& smppDescriptor)
{
    Operation * operation = 0;
    // uint16_t UMR;

    if(session.getLongCallContext().continueExec)
    {
        smsc_log_debug(logger,"continue execution. operation_id=%u", unsigned(command.getOperationId()));
//        operation = session.setCurrentOperation(command.getOperationId());
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
        smsc_log_debug(logger,"Session: process USSD_DIALOG operation");

        if (smppDescriptor.wantOpenUSSD)
        {
            // operation = session.AddNewOperationToHash(command, CO_USSD_DIALOG);
            operation = session.createOperation(command, CO_USSD_DIALOG);
            if ( command.flagSet(transport::smpp::SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG))
                operation->setFlag(sessions::OperationFlags::SERVICE_INITIATED_USSD_DIALOG);
            operation->setStatus(sessions::OPERATION_INITED);
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



void SmppEventHandler::ModifyOperationBeforeExecuting(Session& session, SmppCommand& command, CSmppDescriptor& smppDescriptor)
{
    if (smppDescriptor.isResp)
        ProcessModifyRespCommandOperation(session, command, smppDescriptor);
    else
        ProcessModifyCommandOperation(session, command, smppDescriptor);
}


void SmppEventHandler::ModifyOperationAfterExecuting(Session& session, SmppCommand& command, RuleStatus& ruleStatus, CSmppDescriptor& smppDescriptor)
{
    Operation * currentOperation = session.getCurrentOperation();
    if (!currentOperation) throw SCAGException("Session: Fatal error - cannot end operation. Couse: current operation not found");

    if ( ruleStatus.status == STATUS_FAILED )
    {
        session.closeCurrentOperation(); 
        return;
    }

    if (ruleStatus.status == STATUS_LONG_CALL)
        return;

    smsc_log_debug(logger, "current operation=%p status=%d(%s)", currentOperation, currentOperation->getStatus(), currentOperation->getNamedStatus());

    switch (smppDescriptor.cmdType)
    {
    case CO_DELIVER:
    case CO_DATA_SC_2_SME:
    case CO_RECEIPT:
        if (currentOperation->getStatus() == OPERATION_COMPLETED) session.closeCurrentOperation();
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
#endif // if 0


void SmppEventHandler::process( SCAGCommand& command, Session& session, RuleStatus& rs )
{
    smsc_log_debug(logger, "Process EventHandler...");

    SmppCommand& smppcommand = static_cast<SmppCommand&>(command);
    SmppCommandAdapter _command(smppcommand);

    Infrastructure& istr = BillingManager::Instance().getInfrastructure();

    const Address abonentAddr( session.sessionKey().toString().c_str() );
    CSmppDescriptor smppDescriptor = CommandBridge::getSmppDescriptor(smppcommand);

    int providerId = istr.GetProviderID(command.getServiceId());
    if (providerId == 0) 
    {
        if (smppDescriptor.isResp) session.closeCurrentOperation();
        throw SCAGException("SmppEventHandler: Cannot find ProviderID for ServiceID=%d", command.getServiceId());
    }

    int operatorId = istr.GetOperatorID(abonentAddr), hi = propertyObject.HandlerId;
    if (operatorId == 0) 
    {
        RegisterAlarmEvent(1, abonentAddr.toString(), CommandBridge::getProtocolForEvent(smppcommand), command.getServiceId(),
                           providerId, 0, 0, session.sessionPrimaryKey(),
                           (hi == EH_SUBMIT_SM)||(hi == EH_DELIVER_SM) ? 'I' : 'O');
        
        if (smppDescriptor.isResp) session.closeCurrentOperation();
        throw SCAGException("SmppEventHandler: Cannot find OperatorID for %s abonent", abonentAddr.toString().c_str());
    }

    SMS& sms = CommandBridge::getSMS(smppcommand);
    int msgRef = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ? sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE):-1;

    /*uint32_t commandStatus = (smppcommand->cmdid == DELIVERY_RESP || smppcommand->cmdid == SUBMIT_RESP || smppcommand->cmdid == DATASM_RESP) 
			    ? smppcommand->get_resp()->status : smppcommand->status;*/
    Property routeId;
    routeId.setStr(sms.getRouteId());
    CommandProperty commandProperty( &command, smppcommand.get_status(), abonentAddr, providerId, operatorId,
                                     smppcommand.getServiceId(), msgRef, smppDescriptor.cmdType, routeId);

    if(!session.getLongCallContext().continueExec)
        RegisterTrafficEvent( commandProperty, session.sessionPrimaryKey(),
                              (hi == EH_SUBMIT_SM) || 
                              (hi == EH_DELIVER_SM) ||
                              (hi == EH_DATA_SM) ?
                              CommandBridge::getMessageBody(smppcommand) : "" );
    
    /*
    try {
        ModifyOperationBeforeExecuting(session, smppcommand, smppDescriptor);
    } catch (SCAGException& e)
    {
        smsc_log_error(logger, "EventHandler cannot start/locate operation. Details: %s", e.what());
        rs.result = smsc::system::Status::SMDELIFERYFAILURE;
        rs.status = STATUS_FAILED;
        return;
    }
     */

    ActionContext* actionContext = 0;
    if(session.getLongCallContext().continueExec) {
    	actionContext = session.getLongCallContext().getActionContext();
    	if (actionContext) {
	        actionContext->resetContext(&(RuleEngine::Instance().getConstants()), 
	                    				&session, &_command, &commandProperty, &rs);
    	} else {
	        smsc_log_error(logger, "EventHandler cannot get actionContext to continue");
    	    rs.result = smsc::system::Status::SMDELIFERYFAILURE;
    	    rs.status = STATUS_FAILED;
    	    return;
    	}
    } else {
    	actionContext = new ActionContext(&(RuleEngine::Instance().getConstants()), 
                    					  &session, &_command, &commandProperty, &rs);
    	session.getLongCallContext().setActionContext(actionContext);
    }
    
    try {
        RunActions(*actionContext);
    } catch (SCAGException& e) {
        smsc_log_error(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = STATUS_FAILED;
    } catch (std::exception& e) {
        smsc_log_error(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = STATUS_FAILED;
    }

    if ( smppcommand.get_status() > 0) {
        rs.result = smppcommand.get_status();
        rs.status = STATUS_FAILED;
        smsc_log_debug( logger, "Command status=%d(%x) overrides RE status", rs.result, rs.result );
    }

    // ModifyOperationAfterExecuting(session, smppcommand, rs, smppDescriptor);
    return;
}

int SmppEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "submit_sm")             return EH_SUBMIT_SM;
    if (str == "submit_sm_resp")        return EH_SUBMIT_SM_RESP;
    if (str == "deliver_sm")            return EH_DELIVER_SM;
    if (str == "deliver_sm_resp")       return EH_DELIVER_SM_RESP;
    if (str == "receipt")               return EH_RECEIPT;

    if (str == "data_sm")               return EH_DATA_SM;
    if (str == "data_sm_resp")          return EH_DATA_SM_RESP;

    return UNKNOWN; 
}

}}
