#include "HttpEventHandler.h"
#include "HttpAdapter.h"
#include "scag/re/actions/ActionContext.h"
#include "scag/re/CommandBrige.h"

namespace scag { namespace re { namespace http {

using namespace scag::transport::http;

RuleStatus HttpEventHandler::processRequest(HttpCommand& command, Session& session)
{
    smsc_log_debug(logger, "Process HttpEventHandler Request...");

    Hash<Property> _constants;
    RuleStatus rs;

    HttpCommandAdapter _command(command);

    try{

        session.setOperationFromPending(command, CO_HTTP_DELIVERY);
        

        time_t now;
        time(&now);

        PendingOperation pendingOperation;
        pendingOperation.type = CO_HTTP_DELIVERY;
        pendingOperation.validityTime = now + SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;
        session.addPendingOperation(pendingOperation);

        CommandProperty commandProperty(command, 0, Address(command.getAbonent().c_str()));

        std::string str;
        RegisterTrafficEvent(commandProperty, session.getPrimaryKey(), str);

        ActionContext context(_constants, session, _command, commandProperty);

        rs = RunActions(context);
        if(rs.result < 0)
            session.closeCurrentOperation();            

        return rs;
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process request command - %s", e.what());
        //TODO: ����� � �����-������
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

    try{
        session.setCurrentOperation(command.getOperationId());

        CommandProperty commandProperty(command, 0, Address(command.getAbonent().c_str()));

        std::string str;
        RegisterTrafficEvent(commandProperty, session.getPrimaryKey(), str);

        ActionContext context(_constants, session, _command, commandProperty);

        rs = RunActions(context);
        if(rs.result < 0)
            session.closeCurrentOperation();            

        return rs;
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process request command - %s", e.what());
        //TODO: ����� � �����-������
    }

    rs.status = false;
    rs.result = -1;
    return rs;
}

RuleStatus HttpEventHandler::processDelivery(HttpCommand& command, Session& session)
{
    smsc_log_debug(logger, "Process HttpEventHandler Delivery...");

    try{
        session.setCurrentOperation(command.getOperationId());
        session.closeCurrentOperation();            
    } catch (SCAGException& e)
    {
        smsc_log_debug(logger, "HttpEventHandler: cannot process request command - %s", e.what());
        //TODO: ����� � �����-������
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

//    CommandProperty commandProperty(command, 0, Address(command.getAbonent()));
//    RegisterTrafficEvent(commandProperty, session.getPrimaryKey(), "");

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
    if (str == "request")   return EH_HTTP_REQUEST;
    if (str == "response")  return EH_HTTP_RESPONSE;
    if (str == "delivery")  return EH_HTTP_DELIVERY;
    return UNKNOWN; 
}

}}}
