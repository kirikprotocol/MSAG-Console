#include "HttpEventHandler.h"
#include "HttpAdapter.h"
#include "scag/re/actions/ActionContext.h"

namespace scag { namespace re { namespace http {

using namespace scag::transport::http;

RuleStatus HttpEventHandler::processRequest(HttpCommand& command, Session& session)
{
    smsc_log_debug(logger, "Process HttpEventHandler Request...");

    Hash<Property> _constants;
    RuleStatus rs;

    HttpCommandAdapter _command(command);

//    SACC_TRAFFIC_INFO_EVENT_t ev;
//    CommandBrige::makeTrafficEvent(*smppcommand, (int)propertyObject.HandlerId, session.getPrimaryKey(), ev);
//    Statistics::Instance().registerSaccEvent(ev);
   
    try{

        session.setOperationFromPending(command, HTTP_DELIVERY);

        time_t now;
        time(&now);

        PendingOperation pendingOperation;
        pendingOperation.type = HTTP_DELIVERY;
        pendingOperation.validityTime = now + SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;
        session.addPendingOperation(pendingOperation);

        ActionContext context(_constants, session, _command, command.getServiceId(), Address(command.getAbonent().c_str()));

        rs = RunActions(context);
        if(rs.result < 0)
            session.closeCurrentOperation();            

        return rs;
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process request command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }

    rs.status = false;
    rs.result = -1;
    return rs;
}

RuleStatus HttpEventHandler::processResponse(HttpCommand& command, Session& session)
{
    smsc_log_debug(logger, "Process HttpEventHandler Response...");

    Hash<Property> _constants;
    RuleStatus rs;

    HttpCommandAdapter _command(command);

//    SACC_TRAFFIC_INFO_EVENT_t ev;
//    CommandBrige::makeTrafficEvent(*smppcommand, (int)propertyObject.HandlerId, session.getPrimaryKey(), ev);
//    Statistics::Instance().registerSaccEvent(ev);
   
    try{
        session.setCurrentOperation(command.getOperationId());

        ActionContext context(_constants, session, _command, command.getServiceId(), Address(command.getAbonent().c_str()));

        rs = RunActions(context);
        if(rs.result < 0)
            session.closeCurrentOperation();            

        return rs;
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process request command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }

    rs.status = false;
    rs.result = -1;
    return rs;
}

RuleStatus HttpEventHandler::processDelivery(HttpCommand& command, Session& session)
{
    smsc_log_debug(logger, "Process HttpEventHandler Delivery...");

//    SACC_TRAFFIC_INFO_EVENT_t ev;
//    CommandBrige::makeTrafficEvent(*smppcommand, (int)propertyObject.HandlerId, session.getPrimaryKey(), ev);
//    Statistics::Instance().registerSaccEvent(ev);
   
    try{
        session.setCurrentOperation(command.getOperationId());
        session.closeCurrentOperation();            
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process request command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }

    RuleStatus rs;

    rs.status = false;
    rs.result = -1;
    return rs;
}

RuleStatus HttpEventHandler::process(SCAGCommand& command, Session& session)
{
    smsc_log_debug(logger, "Process HttpEventHandler...");

    HttpCommand& hc = (HttpCommand&)command;

    switch(hc.getCommandId())
    {
        case HTTP_REQUEST:
            return processRequest(hc, session);
        case HTTP_RESPONSE:
            return processResponse(hc, session);
        case HTTP_DELIVERY:
            return processDelivery(hc, session);
        default:
            smsc_log_debug(logger, "HttpEventHandler: unknown command");
    }

    return RuleStatus();
}

int HttpEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "request")   return HTTP_REQUEST;
    if (str == "response")  return HTTP_RESPONSE;
    if (str == "delivery")  return HTTP_DELIVERY;
    return UNKNOWN; 
}

}}}
