#include "HttpEventHandler.h"
#include "HttpAdapter.h"
#include "scag/re/actions/ActionContext.h"
#include "scag/re/CommandBrige.h"
#include "scag/sessions/SessionManager.h"
#include "scag/re/RuleEngine.h"

namespace scag { namespace re { namespace http {

using namespace scag::transport::http;
using namespace scag::sessions;

void HttpEventHandler::processRequest(HttpRequest& command, ActionContext& context)
{
    smsc_log_debug(logger, "Process HttpEventHandler Request...");

    RuleStatus& rs = context.getRuleStatus();
    Session& session = context.getSession();

    try{
        if(command.isInitial())
            session.AddNewOperationToHash(command, CO_HTTP_DELIVERY);        
        else            
            session.setOperationFromPending(command, CO_HTTP_DELIVERY);

        RunActions(context);
        if(rs.status == STATUS_FAILED) session.closeCurrentOperation();

        PendingOperation pendingOperation;
        pendingOperation.type = CO_HTTP_DELIVERY;
        pendingOperation.validityTime = time(NULL) + SessionManager::DEFAULT_EXPIRE_INTERVAL;
        session.addPendingOperation(pendingOperation);
        return;
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process request command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }
   
    session.closeCurrentOperation();

    rs.status = STATUS_FAILED;
    rs.result = -1;
    return;
}

void HttpEventHandler::processResponse(HttpResponse& command, ActionContext& context)
{
    smsc_log_debug(logger, "Process HttpEventHandler Response...");

    RuleStatus& rs = context.getRuleStatus();
    Session& session = context.getSession();

    try{
        session.setCurrentOperation(command.getOperationId());

        RunActions(context);
        if(rs.status == STATUS_FAILED) session.closeCurrentOperation();

        return;
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process response command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }
    session.closeCurrentOperation();

    rs.status = STATUS_FAILED;
    rs.result = -1;
    return;
}

void HttpEventHandler::processDelivery(HttpResponse& command, ActionContext& context)
{
    smsc_log_debug(logger, "Process HttpEventHandler Delivery...");

    RuleStatus& rs = context.getRuleStatus();
    Session& session = context.getSession();

    try{
        session.setCurrentOperation(command.getOperationId());

        RunActions(context);

        session.closeCurrentOperation();

        return;
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process delivery command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }

    session.closeCurrentOperation();

    rs.status = STATUS_FAILED;
    rs.result = -1;
    return;
}

void HttpEventHandler::process(SCAGCommand& command, Session& session, RuleStatus& rs)
{
    smsc_log_debug(logger, "Process HttpEventHandler...");

    HttpCommand& hc = (HttpCommand&)command;

    Infrastructure& istr = BillingManager::Instance().getInfrastructure();

    const Address& abonentAddr = session.getSessionKey().abonentAddr;

    uint32_t operatorId = istr.GetOperatorID(abonentAddr);
    if (operatorId == 0)
    {
        RegisterAlarmEvent(1, abonentAddr.toString(), PROTOCOL_HTTP, hc.getServiceId(),
                            hc.getProviderId(), 0, 0, session.getPrimaryKey(),
                            hc.getCommandId() == HTTP_RESPONSE ? 'O' : 'I');
        
        throw SCAGException("HttpEventHandler: Cannot find OperatorID for %s abonent", abonentAddr.toString().c_str());
    }

    smsc_log_debug(logger, "HttpEventHandler: Operator found. Id = %d", operatorId);

/*    uint32_t providerId = istr.GetProviderID(hc.getServiceId());
    if (providerId == 0) 
        throw SCAGException("HttpEventHandler: Cannot find ProviderID for ServiceID=%d", hc.getServiceId());*/

    smsc_log_debug(logger, "HttpEventHandler: Provider ID found. ID = %d", hc.getProviderId());

    CommandProperty cp(&command, 0, abonentAddr, hc.getProviderId(), operatorId, hc.getServiceId(), -1, CO_HTTP_DELIVERY);

    HttpCommandAdapter _command(hc);
    ActionContext context(RuleEngine::Instance().getConstants(), session, &_command, cp, rs);

    if(!session.getLongCallContext().continueExec)
        RegisterTrafficEvent(cp, session.getPrimaryKey(), "");

    switch(hc.getCommandId())
    {
        case HTTP_REQUEST:
            return processRequest((HttpRequest&)hc, context);
        case HTTP_RESPONSE:
            return processResponse((HttpResponse&)hc, context);
        case HTTP_DELIVERY:
            return processDelivery((HttpResponse&)hc, context);
        default:
            smsc_log_debug(logger, "HttpEventHandler: unknown command");
    }

    rs.status = STATUS_FAILED;
    rs.result = -1;
    return;
}

int HttpEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "request")   return EH_HTTP_REQUEST;
    if (str == "response")  return EH_HTTP_RESPONSE;
    if (str == "delivery")  return EH_HTTP_DELIVERY;
    return UNKNOWN; 
}

}}}
