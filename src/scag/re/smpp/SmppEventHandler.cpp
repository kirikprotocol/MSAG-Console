#include "SmppEventHandler.h"
#include "scag/re/smpp/SmppAdapter.h"
#include "scag/re/actions/ActionContext.h"
#include "scag/re/CommandBrige.h"
#include "scag/re/RuleEngine.h"

namespace scag { namespace re {

using namespace scag::re::smpp;

void SmppEventHandler::StartOperation(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor)
{
    Operation * operation = 0;
    int16_t UMR;

    switch (smppDiscriptor.cmdType)
    {
    case CO_DELIVER:
    case CO_DATA_SC_2_SME:
        if ((smppDiscriptor.currentIndex <= 1)&&(!smppDiscriptor.isResp))
        {
            operation = session.AddNewOperationToHash(command, smppDiscriptor.cmdType);
            operation->receiveNewPart(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        } 
        else 
        {
            operation = session.setCurrentOperation(command.getOperationId());

            if (!smppDiscriptor.isResp)
                operation->receiveNewPart(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
            else 
                operation->receiveNewResp(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        }

        break;

    
    case CO_SUBMIT:
    case CO_DATA_SME_2_SC:
        //TODO: разобаться с RECEIPT`ом и SUBMIT_RESP

        UMR = session.getUSR();

        if ((UMR == 0)&&(!smppDiscriptor.isResp))
        {
            if (smppDiscriptor.currentIndex == 0) 
                operation = session.AddNewOperationToHash(command, smppDiscriptor.cmdType);
            else
                operation = session.setCurrentOperation(command.getOperationId());

            operation->receiveNewPart(smppDiscriptor.currentIndex, smppDiscriptor.lastIndex);

            break;
        }

        if (smppDiscriptor.isResp) 
        {
            operation = session.setCurrentOperation(command.getOperationId());
            operation->receiveNewResp(smppDiscriptor.currentIndex, smppDiscriptor.lastIndex);
            break;
        }

        if (smppDiscriptor.currentIndex == 0)
        {
            session.setOperationFromPending(command, smppDiscriptor.cmdType);
            break;
        } 

        operation = session.setCurrentOperation(command.getOperationId());

        operation->receiveNewPart(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        break;

    case CO_RECEIPT:
        //TODO:: Нужно учесть политику для multipart

        if (!smppDiscriptor.isResp) 
        {
            operation->receiveNewPart(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
            break;
        }

        operation->receiveNewResp(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        break;   

    case CO_USSD_DIALOG:
        smsc_log_debug(logger,"Session: process USSD_DIALOG operation");

        if (smppDiscriptor.isResp)
        {
            operation = session.setCurrentOperationByType(CO_USSD_DIALOG);

            if (smppDiscriptor.isUSSDClosed) operation->setStatus(OPERATION_COMPLETED);
            else operation->setStatus(OPERATION_CONTINUED);
            break;
        }

        if (smppDiscriptor.wantOpenUSSD)
        {
             operation = session.AddNewOperationToHash(command, CO_USSD_DIALOG);
             operation->setStatus(OPERATION_INITED);
             break;
        }

        operation = session.setCurrentOperationByType(CO_USSD_DIALOG);
        operation->setStatus(OPERATION_CONTINUED);

        break;
    }
}

void SmppEventHandler::EndOperation(Session& session, SmppCommand& command, RuleStatus& ruleStatus, CSmppDiscriptor& smppDiscriptor)
{
    Operation * currentOperation = session.GetCurrentOperation();
    if (!currentOperation) throw SCAGException("Session: Fatal error - cannot end operation. Couse: current operation not found");

    if (!ruleStatus.status) 
    {
        session.closeCurrentOperation(); 
        return;
    }
    
    smsc_log_debug(logger, "current operation status=%d",currentOperation->getStatus());

    switch (smppDiscriptor.cmdType)
    {
    case CO_DELIVER:
    case CO_DATA_SC_2_SME:
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


    case CO_RECEIPT:
        if (currentOperation->getStatus() == OPERATION_COMPLETED) session.closeCurrentOperation();

        break;      

    case CO_USSD_DIALOG:
        if ((smppDiscriptor.isUSSDClosed)&&(smppDiscriptor.isResp)) session.closeCurrentOperation();
        break;
    }
}



RuleStatus SmppEventHandler::process(SCAGCommand& command, Session& session)
{
    smsc_log_debug(logger, "Process EventHandler...");

    Hash<Property> _constants = RuleEngine::Instance().getConstants();
    RuleStatus rs;
   

    smsc::util::config::Config config;
    
    
    SmppCommand * smppcommand = dynamic_cast<SmppCommand *>(&command);
    if (!smppcommand) throw SCAGException("SmppEventHandler: command is not 'smpp-type'");

    SmppCommandAdapter _command(*smppcommand);

    /////////////////////////////////////////

    Infrastructure& istr = BillingManager::Instance().getInfrastructure();

    Address& abonentAddr = CommandBrige::getAbonentAddr(*smppcommand);
    CSmppDiscriptor smppDiscriptor = CommandBrige::getSmppDiscriptor(*smppcommand);

    int providerId = istr.GetProviderID(command.getServiceId());
    if (providerId == 0) 
        throw SCAGException("SmppEventHandler: Cannot find ProviderID for ServiceID=%d", command.getServiceId());

    int operatorId = istr.GetOperatorID(abonentAddr);
    if (operatorId == 0) 
    {
        RegisterAlarmEvent(1, abonentAddr.toString(), CommandBrige::getProtocolForEvent(*smppcommand), command.getServiceId(),
                            providerId, 0, 0, session.getPrimaryKey().abonentAddr.toString(),
                            (propertyObject.HandlerId == EH_SUBMIT_SM)||(propertyObject.HandlerId == EH_DELIVER_SM) ? 'I' : 'O');
        
        throw SCAGException("SmppEventHandler: Cannot find OperatorID for %s abonent", abonentAddr.toString().c_str());
    }
    
    CommandProperty commandProperty(command, (*smppcommand)->status, abonentAddr, providerId, operatorId, smppDiscriptor.cmdType);

    std::string message = CommandBrige::getMessageBody(*smppcommand);

    RegisterTrafficEvent(commandProperty, session.getPrimaryKey(), message);
    
    /////////////////////////////////////////

    if ((*smppcommand)->status > 0)
    {
        rs.result = (*smppcommand)->status;
        rs.status = true;
    }
    

    try {
        StartOperation(session, *smppcommand, smppDiscriptor);
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "EventHandler cannot start/locate operation. Details: %s", e.what());
        return rs;
    }
   
    ActionContext context(_constants, session, _command, commandProperty);

    try
    {
        rs = RunActions(context);
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = false;
    } catch (std::exception& e)
    {
        smsc_log_debug(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = false;
    }

    EndOperation(session, *smppcommand, rs, smppDiscriptor);
    return rs;
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
