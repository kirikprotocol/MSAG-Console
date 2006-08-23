#include "HttpEventHandler.h"
#include "HttpAdapter.h"
#include "scag/re/actions/ActionContext.h"
#include "scag/re/CommandBrige.h"

namespace scag { namespace re { namespace http {

using namespace scag::transport::http;

RuleStatus HttpEventHandler::processRequest(HttpRequest& command, Session& session, CommandProperty& commandProperty)
{
    smsc_log_debug(logger, "Process HttpEventHandler Request...");

    Hash<Property> _constants;
    RuleStatus rs;

    HttpCommandAdapter _command(command);

    try{
        if(command.isInitial())
            session.AddNewOperationToHash(command, CO_HTTP_DELIVERY);        
        else            
            session.setOperationFromPending(command, CO_HTTP_DELIVERY);

        RegisterTrafficEvent(commandProperty, session.getPrimaryKey(), "");

        ActionContext context(_constants, session, _command, commandProperty);

        rs = RunActions(context);
        if(!rs.status) session.closeCurrentOperation();

        PendingOperation pendingOperation;
        pendingOperation.type = CO_HTTP_DELIVERY;
        pendingOperation.validityTime = time(NULL) + SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;
        session.addPendingOperation(pendingOperation);
        return rs;
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process request command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }
    
    session.closeCurrentOperation();

    rs.status = false;
    rs.result = -1;
    return rs;
}

RuleStatus HttpEventHandler::processResponse(HttpResponse& command, Session& session, CommandProperty& commandProperty)
{
    smsc_log_debug(logger, "Process HttpEventHandler Response...");

    Hash<Property> _constants;
    RuleStatus rs;

    HttpCommandAdapter _command(command);

    try{
        session.setCurrentOperation(command.getOperationId());

        RegisterTrafficEvent(commandProperty, session.getPrimaryKey(), "");

        ActionContext context(_constants, session, _command, commandProperty);

        rs = RunActions(context);
        if(!rs.status) session.closeCurrentOperation();

        return rs;
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process response command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }

    session.closeCurrentOperation();

    rs.status = false;
    rs.result = -1;
    return rs;
}

RuleStatus HttpEventHandler::processDelivery(HttpResponse& command, Session& session, CommandProperty& cp)
{
    smsc_log_debug(logger, "Process HttpEventHandler Delivery...");

    Hash<Property> _constants;
    RuleStatus rs;

    HttpCommandAdapter _command(command);

    try{
        session.setCurrentOperation(command.getOperationId());

        RegisterTrafficEvent(cp, session.getPrimaryKey(), "");

        ActionContext context(_constants, session, _command, cp);

        rs = RunActions(context);

        session.closeCurrentOperation();

        return rs;
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process delivery command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }

    session.closeCurrentOperation();

    rs.status = false;
    rs.result = -1;
    return rs;
}

RuleStatus HttpEventHandler::process(SCAGCommand& command, Session& session)
{
    smsc_log_debug(logger, "Process HttpEventHandler...");

    HttpCommand& hc = (HttpCommand&)command;

//    CommandProperty commandProperty(command, 0, Address(command.getAbonent()));
//    RegisterTrafficEvent(commandProperty, session.getPrimaryKey(), "");

    Infrastructure& istr = BillingManager::Instance().getInfrastructure();

    Address abonentAddr(hc.getAddress().c_str());

    uint32_t operatorId = istr.GetOperatorID(abonentAddr);
    if (operatorId == 0)
    {
        RegisterAlarmEvent(1, abonentAddr.toString(), PROTOCOL_HTTP, hc.getServiceId(),
                            hc.getProviderId(), 0, 0, session.getPrimaryKey().abonentAddr.toString(),
                            hc.getCommandId() == HTTP_RESPONSE ? 'O' : 'I');
        
        throw SCAGException("HttpEventHandler: Cannot find OperatorID for %s abonent", abonentAddr.toString().c_str());
    }

    smsc_log_debug(logger, "HttpEventHandler: Operator found. Id = %d", operatorId);

/*    uint32_t providerId = istr.GetProviderID(hc.getServiceId());
    if (providerId == 0) 
        throw SCAGException("HttpEventHandler: Cannot find ProviderID for ServiceID=%d", hc.getServiceId());*/

    smsc_log_debug(logger, "HttpEventHandler: Provider ID found. ID = %d", hc.getProviderId());

    CommandProperty cp(command, 0, abonentAddr, hc.getProviderId(), operatorId, CO_HTTP_DELIVERY);

    switch(hc.getCommandId())
    {
        case HTTP_REQUEST:
            return processRequest((HttpRequest&)hc, session, cp);
        case HTTP_RESPONSE:
            return processResponse((HttpResponse&)hc, session, cp);
        case HTTP_DELIVERY:
            return processDelivery((HttpResponse&)hc, session, cp);
        default:
            smsc_log_debug(logger, "HttpEventHandler: unknown command");
    }

    RuleStatus rs;
    
    rs.status = false;
    rs.result = -1;
    return rs;
}

int HttpEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "request")   return EH_HTTP_REQUEST;
    if (str == "response")  return EH_HTTP_RESPONSE;
    if (str == "delivery")  return EH_HTTP_DELIVERY;
    return UNKNOWN; 
}

}}}
