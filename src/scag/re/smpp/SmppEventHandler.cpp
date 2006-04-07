#include "SmppEventHandler.h"
#include "scag/re/smpp/SmppAdapter.h"
#include "scag/re/actions/ActionContext.h"
#include "scag/re/CommandBrige.h"

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
        //TODO: разобатьс€ с RECEIPT`ом и SUBMIT_RESP

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
        //TODO: проверить есть ли ожидаема€ операци€ SUBMIT, если еЄ нет, то что и как делать?

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
            operation = session.setCurrentOperation(command.getOperationId());

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

        operation = session.setCurrentOperation(command.getOperationId());
        operation->setStatus(OPERATION_CONTINUED);

        break;
    }
}

void SmppEventHandler::EndOperation(Session& session, SmppCommand& command, RuleStatus& ruleStatus)
{
    Operation * currentOperation = session.GetCurrentOperation();
    if (!currentOperation) throw SCAGException("Session: Fatal error - cannot end operation. Couse: current operation not found");

    CSmppDiscriptor smppDiscriptor = CommandBrige::getSmppDiscriptor(command);

    switch (smppDiscriptor.cmdType)
    {
    case CO_DELIVER:
        if ((currentOperation->getStatus() == OPERATION_COMPLETED)||(!ruleStatus.result)) session.closeCurrentOperation();
        break;

    case CO_SUBMIT:

        if ((currentOperation->getStatus() == OPERATION_COMPLETED)||(!ruleStatus.result)) session.closeCurrentOperation();

        //TODO: узнать заказал ли сервис отчЄт о доставке
        if (!smppDiscriptor.m_isTransact) 
        {
            time_t now;
            time(&now);

            PendingOperation pendingOperation;
            pendingOperation.type = CO_RECEIPT;
            pendingOperation.validityTime = now + SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;

            session.addPendingOperation(pendingOperation);
        }
        break;

/*
    case CO_RECEIPT_DELIVER_SM:
        //TODO:: Ќужно учесть политику дл€ multipart
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

    Hash<Property> _constants;
    RuleStatus rs;

    smsc::util::config::Config config;

    
    SmppCommand * smppcommand = dynamic_cast<SmppCommand *>(&command);
    if (!smppcommand) throw SCAGException("SmppEventHandler: command is not 'smpp-type'");

    SmppCommandAdapter _command(*smppcommand);

    /////////////////////////////////////////

    CommandProperty commandProperty(command, (*smppcommand)->status, CommandBrige::getAbonentAddr(*smppcommand));
    RegisterTrafficEvent(commandProperty, session.getPrimaryKey(), CommandBrige::getMessageBody(*smppcommand));
    
    /////////////////////////////////////////
    
   
    try {
        StartOperation(session, *smppcommand);
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "EventHandler cannot process command. Details: %s", e.what());
        return rs;
        //TODO: отлуп в стейт-машину
    }
   
    ActionContext context(_constants, session, _command, commandProperty);

    rs = RunActions(context);

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
