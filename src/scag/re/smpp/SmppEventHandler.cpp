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
    case CO_DELIVER_SM:
        if ((smppDiscriptor.currentIndex == 0)&&(smppDiscriptor.lastIndex == 0))
            session.AddNewOperationToHash(command, smppDiscriptor.cmdType);
        else
        {
            operation = session.AddNewOperationToHash(command, smppDiscriptor.cmdType);
            operation->receiveNewPart(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        }
        break;

    case CO_DELIVER_SM_RESP:
        operation = session.setCurrentOperation(command.getOperationId());
        operation->receiveNewResp(smppDiscriptor.currentIndex, smppDiscriptor.lastIndex);
        break;

    case CO_SUBMIT_SM:
        UMR = CommandBrige::getUMR(command);

        if (UMR == 0)
        {
            if (smppDiscriptor.currentIndex == 0) 
                session.AddNewOperationToHash(command, smppDiscriptor.cmdType);
            else
                session.setCurrentOperation(command.getOperationId());
            break;
        }
        //TODO: проверить есть ли ожидаемая операция SUBMIT, если её нет, то что и как делать?

        if (smppDiscriptor.currentIndex == 0)
            session.setOperationFromPending(command, smppDiscriptor.cmdType);
        else
        {
            operation = session.setCurrentOperation(command.getOperationId());
            operation->receiveNewPart(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        }
        break;

    case CO_SUBMIT_SM_RESP:
        operation = session.setCurrentOperation(command.getOperationId());
        operation->receiveNewResp(smppDiscriptor.currentIndex,smppDiscriptor.lastIndex);
        break;

    case CO_RECEIPT_DELIVER_SM:
        session.setOperationFromPending(command, smppDiscriptor.cmdType);
        break;

    case CO_USSD_DELIVER:
        smsc_log_debug(logger,"Session: process USSD_DELIVER");

        if (smppDiscriptor.wantOpenUSSD) session.AddNewOperationToHash(command, CO_USSD_DELIVER);
        break;

    case CO_USSD_SUBMIT:
        smsc_log_debug(logger,"Session: process USSD_SUBMIT");

        if (smppDiscriptor.wantOpenUSSD) 
            session.AddNewOperationToHash(command, CO_USSD_SUBMIT);
        break;

    case CO_USSD_DELIVER_RESP:
    case CO_USSD_SUBMIT_RESP:
        break;

    }
}

void SmppEventHandler::EndOperation(Session& session, SmppCommand& command)
{
    Operation * currentOperation = session.GetCurrentOperation();
    if (!currentOperation) throw SCAGException("Session: Fatal error - cannot end operation. Couse: current operation not found");

    CSmppDiscriptor smppDiscriptor = CommandBrige::getSmppDiscriptor(command);

    switch (smppDiscriptor.cmdType)
    {
    case CO_DELIVER_SM:
        break;

    case CO_DELIVER_SM_RESP:
        if ((smppDiscriptor.lastIndex == 0)||((smppDiscriptor.lastIndex > 0)&&(currentOperation->hasReceivedAllResp())))
            session.closeCurrentOperation();
        break;

    case CO_SUBMIT_SM:

        if (!smppDiscriptor.m_isTransact) 
        {
            time_t now;
            time(&now);

            PendingOperation pendingOperation;
            pendingOperation.type = CO_RECEIPT_DELIVER_SM;
            pendingOperation.validityTime = now + SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;

            session.addPendingOperation(pendingOperation);
        }
        break;

    case CO_SUBMIT_SM_RESP:
        
        if ((smppDiscriptor.lastIndex = 0)||((smppDiscriptor.lastIndex > 0)&&(currentOperation->hasReceivedAllResp())))
            session.closeCurrentOperation();
        break;

    case CO_RECEIPT_DELIVER_SM:
        //TODO:: Нужно учесть политику для multipart
        session.closeCurrentOperation();
        break;

    case CO_USSD_DELIVER_RESP:
            smsc_log_debug(logger,"Session: finish process USSD_DELIVER_RESP");
            if (smppDiscriptor.isUSSDClosed) session.closeCurrentOperation();
            break;

    case CO_USSD_SUBMIT_RESP:
            smsc_log_debug(logger,"Session: finish process USSD_SUBMIT_RESP");
            if (smppDiscriptor.isUSSDClosed) session.closeCurrentOperation();
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

    SACC_TRAFFIC_INFO_EVENT_t ev;

    CommandBrige::makeTrafficEvent(*smppcommand, (int)propertyObject.HandlerId, session.getPrimaryKey(), ev);
    Statistics::Instance().registerSaccEvent(ev);

    /////////////////////////////////////////
    
   
    try {
        StartOperation(session, *smppcommand);
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "EventHandler: cannot process command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }
   

    ActionContext context(_constants, session, _command, command.getServiceId(), CommandBrige::getAbonentAddr(*smppcommand));

    rs = RunActions(context);

    EndOperation(session, *smppcommand);

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
