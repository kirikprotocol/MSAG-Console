#include "SmppEventHandler.h"
#include "scag/re/smpp/SmppAdapter.h"
#include "scag/re/actions/ActionContext.h"
#include "scag/re/CommandBrige.h"
#include "scag/re/RuleEngine.h"

namespace scag { namespace re {

using namespace scag::re::smpp;

void SmppEventHandler::ProcessModifyRespCommandOperation(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor)
{
    Operation * operation = 0;

    if(session.getLongCallContext().continueExec)
    {
        smsc_log_debug(logger,"resp: continue execution. operation id=%lld", command.getOperationId());
//        operation = session.setCurrentOperation(command.getOperationId());
        return;
    }

    switch (smppDiscriptor.cmdType)
    {
    case CO_DELIVER:
    case CO_SUBMIT:
    case CO_DATA_SC_2_SME:
    case CO_DATA_SME_2_SC:
    case CO_RECEIPT:
        operation = session.setCurrentOperation(command.getOperationId());
        operation->receiveNewResp(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        break;
    case CO_USSD_DIALOG:
        smsc_log_debug(logger,"Session: process USSD_DIALOG operation");
        operation = session.setCurrentOperationByType(CO_USSD_DIALOG);

        if (smppDiscriptor.isUSSDClosed) operation->setStatus(OPERATION_COMPLETED);
        else operation->setStatus(OPERATION_CONTINUED);

        break;
    }
}

void SmppEventHandler::ProcessModifyCommandOperation(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor)
{
    Operation * operation = 0;
    uint16_t UMR;

    if(session.getLongCallContext().continueExec)
    {
        smsc_log_debug(logger,"continue execution. operation_id=%lld", command.getOperationId());
//        operation = session.setCurrentOperation(command.getOperationId());
        return;
    }

    switch (smppDiscriptor.cmdType)
    {
    case CO_DELIVER:
    case CO_DATA_SC_2_SME:
        if (session.hasPending()) 
            operation = session.setOperationFromPending(command, smppDiscriptor.cmdType);
        else if (smppDiscriptor.currentIndex <= 1)
            operation = session.AddNewOperationToHash(command, smppDiscriptor.cmdType);
        else 
            operation = session.setCurrentOperation(command.getOperationId());

        operation->receiveNewPart(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        break;

    case CO_SUBMIT:
    case CO_DATA_SME_2_SC:
        //TODO: разобаться с RECEIPT`ом и SUBMIT_RESP

        UMR = session.getUSR();

        if ((UMR <= 0)||(session.getCanOpenSubmitOperation()))
        {
            if (smppDiscriptor.currentIndex == 0) 
                operation = session.AddNewOperationToHash(command, smppDiscriptor.cmdType);
            else
                operation = session.setCurrentOperation(command.getOperationId());
        } 
        else if (smppDiscriptor.currentIndex == 0)
            operation = session.setOperationFromPending(command, smppDiscriptor.cmdType);
        else
            operation = session.setCurrentOperation(command.getOperationId());

        operation->receiveNewPart(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        break;

    case CO_RECEIPT:
        //TODO:: Нужно учесть политику для multipart

        operation = session.setCurrentOperation(command.getOperationId());
        operation->receiveNewResp(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        break;   

    case CO_USSD_DIALOG:
        smsc_log_debug(logger,"Session: process USSD_DIALOG operation");

        if (smppDiscriptor.wantOpenUSSD)
        {
            operation = session.AddNewOperationToHash(command, CO_USSD_DIALOG);
            if(command->flagSet(scag::transport::smpp::SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG))
                operation->setFlag(scag::sessions::OperationFlags::SERVICE_INITIATED_USSD_DIALOG);
            operation->setStatus(OPERATION_INITED);
            break;
        }

        operation = session.setCurrentOperationByType(CO_USSD_DIALOG);
        command.setOperationId(session.getCurrentOperationId());
        operation->setStatus(OPERATION_CONTINUED);

        break;
    }
}



void SmppEventHandler::ModifyOperationBeforeExecuting(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor)
{
   if (smppDiscriptor.isResp)
       ProcessModifyRespCommandOperation(session, command, smppDiscriptor);
   else
       ProcessModifyCommandOperation(session, command, smppDiscriptor);
}

void SmppEventHandler::ModifyOperationAfterExecuting(Session& session, SmppCommand& command, RuleStatus& ruleStatus, CSmppDiscriptor& smppDiscriptor)
{
    Operation * currentOperation = session.GetCurrentOperation();
    if (!currentOperation) throw SCAGException("Session: Fatal error - cannot end operation. Couse: current operation not found");

    if(ruleStatus.status == STATUS_FAILED) 
    {
        session.closeCurrentOperation(); 
        return;
    }

    if(ruleStatus.status == STATUS_LONG_CALL)
        return;

    smsc_log_debug(logger, "current operation status=%d", currentOperation->getStatus());

    switch (smppDiscriptor.cmdType)
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

        if (smppDiscriptor.isResp) break; 

        if (smppDiscriptor.m_waitReceipt)
        {
            /*time_t now;
            time(&now);

            PendingOperation pendingOperation;
            pendingOperation.type = CO_RECEIPT;
            //TODO: Определяем время жизни операции
            pendingOperation.validityTime = now + SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;


            session.addPendingOperation(pendingOperation);*/
            session.AddNewOperationToHash(command, CO_RECEIPT);
        }                    
        break;


    case CO_USSD_DIALOG:
        if ((smppDiscriptor.isUSSDClosed)&&(smppDiscriptor.isResp)) session.closeCurrentOperation();
        break;
    }
}

void SmppEventHandler::process(SCAGCommand& command, Session& session, RuleStatus& rs)
{
    smsc_log_debug(logger, "Process EventHandler...");

    SmppCommand& smppcommand = (SmppCommand&)command;
    SmppCommandAdapter _command(smppcommand);

    Infrastructure& istr = BillingManager::Instance().getInfrastructure();

    const Address& abonentAddr = session.getSessionKey().abonentAddr;
    CSmppDiscriptor smppDiscriptor = CommandBrige::getSmppDiscriptor(smppcommand);

    int providerId = istr.GetProviderID(command.getServiceId());
    if (providerId == 0) 
    {
        if (smppDiscriptor.isResp) session.closeCurrentOperation();
        throw SCAGException("SmppEventHandler: Cannot find ProviderID for ServiceID=%d", command.getServiceId());
    }

    int operatorId = istr.GetOperatorID(abonentAddr), hi = propertyObject.HandlerId;
    if (operatorId == 0) 
    {
        RegisterAlarmEvent(1, abonentAddr.toString(), CommandBrige::getProtocolForEvent(smppcommand), command.getServiceId(),
                            providerId, 0, 0, session.getPrimaryKey(),
                            (hi == EH_SUBMIT_SM)||(hi == EH_DELIVER_SM) ? 'I' : 'O');
        
        if (smppDiscriptor.isResp) session.closeCurrentOperation();
        throw SCAGException("SmppEventHandler: Cannot find OperatorID for %s abonent", abonentAddr.toString().c_str());
    }

    SMS& sms = CommandBrige::getSMS(smppcommand);
    int msgRef = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ? sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE):-1;

    /*uint32_t commandStatus = (smppcommand->cmdid == DELIVERY_RESP || smppcommand->cmdid == SUBMIT_RESP || smppcommand->cmdid == DATASM_RESP) 
			    ? smppcommand->get_resp()->status : smppcommand->status;*/
    Property routeId;
    routeId.setStr(sms.getRouteId());
    CommandProperty commandProperty(&command, smppcommand->getCommandStatus(), abonentAddr, providerId, operatorId,
                                     smppcommand.getServiceId(), msgRef, smppDiscriptor.cmdType, routeId);

    if(!session.getLongCallContext().continueExec)
        RegisterTrafficEvent(commandProperty, session.getPrimaryKey(), 
            (hi == EH_SUBMIT_SM)||(hi == EH_DELIVER_SM)||(hi == EH_DATA_SM) ? CommandBrige::getMessageBody(smppcommand) : "");
    
    try {
        ModifyOperationBeforeExecuting(session, smppcommand, smppDiscriptor);
    } catch (SCAGException& e)
    {
        smsc_log_error(logger, "EventHandler cannot start/locate operation. Details: %s", e.what());
        rs.result = smsc::system::Status::SMDELIFERYFAILURE;
        rs.status = STATUS_FAILED;
        return;
    }

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

    if (smppcommand->status > 0) {
        rs.result = smppcommand->status;
        rs.status = STATUS_FAILED;
    }

    ModifyOperationAfterExecuting(session, smppcommand, rs, smppDiscriptor);
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
