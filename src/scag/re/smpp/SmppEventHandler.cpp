#include "SmppEventHandler.h"
#include "scag/re/smpp/SmppAdapter.h"
#include "scag/re/actions/ActionContext.h"
#include "scag/re/CommandBrige.h"
#include "scag/re/RuleEngine.h"

namespace scag { namespace re {

using namespace scag::re::smpp;

void SmppEventHandler::StartOperation(Session& session, SmppCommand& command)
{
    Operation * operation = 0;
    int UMR;

    CSmppDiscriptor smppDiscriptor = CommandBrige::getSmppDiscriptor(command);

    switch (smppDiscriptor.cmdType)
    {
    case CO_DELIVER:
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
        //TODO: ���������� � RECEIPT`�� � SUBMIT_RESP

        UMR = CommandBrige::getUMR(command);

        if ((UMR == 0)&&(!smppDiscriptor.isResp))
        {
            if (smppDiscriptor.currentIndex == 0) 
                operation = session.AddNewOperationToHash(command, smppDiscriptor.cmdType);
            else
                operation = session.setCurrentOperation(command.getOperationId());

            operation->receiveNewPart(smppDiscriptor.currentIndex, smppDiscriptor.lastIndex);

            break;
        }

        operation = session.setCurrentOperation(command.getOperationId());

        if (smppDiscriptor.isResp) 
        {
            operation->receiveNewResp(smppDiscriptor.currentIndex, smppDiscriptor.lastIndex);
            break;
        }
        //TODO: ��������� ���� �� ��������� �������� SUBMIT, ���� � ���, �� ��� � ��� ������?

        if (smppDiscriptor.currentIndex == 0)
            session.setOperationFromPending(command, CO_SUBMIT);
        else
            operation = session.setCurrentOperation(command.getOperationId());
        operation->receiveNewPart(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);

        break;

  /*  case CO_RECEIPT_DELIVER_SM:
        session.setOperationFromPending(command, smppDiscriptor.cmdType);
        break;   */

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

void SmppEventHandler::EndOperation(Session& session, SmppCommand& command, RuleStatus& ruleStatus)
{
    Operation * currentOperation = session.GetCurrentOperation();
    if (!currentOperation) throw SCAGException("Session: Fatal error - cannot end operation. Couse: current operation not found");

    if (!ruleStatus.result) 
    {
        session.closeCurrentOperation(); 
        return;
    }
    

    CSmppDiscriptor smppDiscriptor = CommandBrige::getSmppDiscriptor(command);

    switch (smppDiscriptor.cmdType)
    {
    case CO_DELIVER:
        if (currentOperation->getStatus() == OPERATION_COMPLETED) session.closeCurrentOperation();
        break;

    case CO_SUBMIT:

        if (currentOperation->getStatus() == OPERATION_COMPLETED) session.closeCurrentOperation();

        //TODO: ������ ������� �� ������ ����� � ��������
       /* if (!smppDiscriptor.m_isTransact) 
        {
            time_t now;
            time(&now);

            PendingOperation pendingOperation;
            pendingOperation.type = CO_RECEIPT;
            pendingOperation.validityTime = now + SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;

            session.addPendingOperation(pendingOperation);
        }                    */
        break;

/*
    case CO_RECEIPT_DELIVER_SM:
        //TODO:: ����� ������ �������� ��� multipart
        session.closeCurrentOperation();
        break;      */

    case CO_USSD_DIALOG:
        //smsc_log_debug(logger,"Session: finish process USSD_DIALOG");
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
    
    int operatorId = 105;
    int providerId = 1;   
/*    
    int operatorId = istr.GetOperatorID(abonentAddr);
    if (operatorId == 0) 
        throw SCAGException("SmppEventHandler: Cannot find OperatorID for %s abonent", abonentAddr.toString().c_str());
    
    int providerId = istr.GetProviderID(command.getServiceId());
    if (providerId == 0) 
        throw SCAGException("SmppEventHandler: Cannot find ProviderID for ServiceID=%d", command.getServiceId());
     */
  
    CommandProperty commandProperty(command, (*smppcommand)->status, abonentAddr, providerId, operatorId);

    std::string message = CommandBrige::getMessageBody(*smppcommand);

    RegisterTrafficEvent(commandProperty, session.getPrimaryKey(), message);
    
    /////////////////////////////////////////

    try {
        StartOperation(session, *smppcommand);
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "EventHandler cannot start/locate operation. Details: %s", e.what());
        return rs;
        //TODO: ����� � �����-������
    }
   
    ActionContext context(_constants, session, _command, commandProperty);

    try
    {
        rs = RunActions(context);
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.result = 0;
    } catch (std::exception& e)
    {
        smsc_log_debug(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.result = 0;
    }

    EndOperation(session, *smppcommand, rs);
    return rs;
}

int SmppEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "submit_sm")             return EH_SUBMIT_SM;
    if (str == "submit_sm_resp")        return EH_SUBMIT_SM_RESP;
    if (str == "deliver_sm")            return EH_DELIVER_SM;
    if (str == "deliver_sm_resp")       return EH_DELIVER_SM_RESP;
    if (str == "receipt")               return EH_RECEIPT;
    return UNKNOWN; 
}

}}
