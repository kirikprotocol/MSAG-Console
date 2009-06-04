#include "HttpEventHandler2.h"
#include "HttpAdapter2.h"
#include "ActionContext2.h"
#include "scag/sessions/base/Operation.h"
#include "RuleEngine2.h"
#include "EventHandlerType.h"

namespace scag2 {
namespace re {
namespace http {

using namespace transport::http;
using namespace sessions;

void HttpEventHandler::processRequest(HttpRequest& command, ActionContext& context, bool isnewevent )
{
    smsc_log_debug(logger, "Process HttpEventHandler Request...");

    RuleStatus& rs = context.getRuleStatus();
    Session& session = context.getSession();

    try{
        if ( command.getOperationId() != invalidOpId() ) {
            // if(session.getLongCallContext().continueExec)
            session.setCurrentOperation(command.getOperationId());
            // else if (command.isInitial())
            // session.AddNewOperationToHash(command, CO_HTTP_DELIVERY);
        } else {
            // session.setOperationFromPending(command, CO_HTTP_DELIVERY);
            session.createOperation( command, CO_HTTP_DELIVERY );
            command.setOperationId( session.getCurrentOperationId() );
        }

        RunActions(context);
        if ( isnewevent ) {
            isnewevent = false;
            setKeywords( context );
        }
        if(rs.status == STATUS_FAILED) session.closeCurrentOperation();

        /*
        if(rs.status != STATUS_LONG_CALL)
        {
            PendingOperation pendingOperation;
            pendingOperation.type = CO_HTTP_DELIVERY;
            pendingOperation.validityTime = time(NULL) + SessionManager::DEFAULT_EXPIRE_INTERVAL;
            session.addPendingOperation(pendingOperation);
        }
         */
        return;
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger, "HttpEventHandler: cannot process request command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }
   
    if ( isnewevent ) {
        isnewevent = false;
        setKeywords( context );
    }

    session.closeCurrentOperation();
    rs.status = STATUS_FAILED;
    rs.result = -1;
    return;
}

void HttpEventHandler::processResponse(HttpResponse& command, ActionContext& context, bool isnewevent)
{
    smsc_log_debug(logger, "Process HttpEventHandler Response...");

    RuleStatus& rs = context.getRuleStatus();
    Session& session = context.getSession();

    try{
        session.setCurrentOperation(command.getOperationId());

        RunActions(context);
        if ( isnewevent ) {
            isnewevent = false;
            setKeywords( context );
        }
        if(rs.status == STATUS_FAILED) session.closeCurrentOperation();

        return;
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger, "HttpEventHandler: cannot process response command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }

    if ( isnewevent ) {
        isnewevent = false;
        setKeywords( context );
    }

    session.closeCurrentOperation();
    rs.status = STATUS_FAILED;
    rs.result = -1;
    return;
}

void HttpEventHandler::processDelivery(HttpResponse& command, ActionContext& context, bool isnewevent)
{
    smsc_log_debug(logger, "Process HttpEventHandler Delivery...");

    RuleStatus& rs = context.getRuleStatus();
    Session& session = context.getSession();

    try{
        session.setCurrentOperation(command.getOperationId());

        RunActions(context);
        if ( isnewevent ) {
            isnewevent = false;
            setKeywords( context );
        }

        if (rs.status != STATUS_LONG_CALL) {
          session.closeCurrentOperation();
        }

        return;
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger, "HttpEventHandler: cannot process delivery command - %s", e.what());
        //TODO: отлуп в стейт-машину
    }

    if ( isnewevent ) {
        isnewevent = false;
        setKeywords( context );
    }
    session.closeCurrentOperation();

    rs.status = STATUS_FAILED;
    rs.result = -1;
    return;
}


void HttpEventHandler::setKeywords( ActionContext& ctx )
{
    const std::string* kw = ctx.getSession().getCurrentOperation() ? ctx.getSession().getCurrentOperation()->getKeywords() : 0;
    ctx.getCommandProperty().keywords = kw ? *kw : "";
}


void HttpEventHandler::process(SCAGCommand& command, Session& session, RuleStatus& rs, CommandProperty& cp, util::HRTiming*)
{
    smsc_log_debug(logger, "Process HttpEventHandler...");
    HttpCommand& hc = (HttpCommand&)command;
    HttpCommandAdapter _command(hc);

    ActionContext* actionContext = 0;
    bool isnewevent = false;
    if(session.getLongCallContext().continueExec) {
      actionContext = session.getLongCallContext().getActionContext();
      if (actionContext) {
          actionContext->resetContext(&(RuleEngine::Instance().getConstants()), 
                      &session, &_command, &cp, &rs);
      } else {
          smsc_log_error(logger, "EventHandler cannot get actionContext to continue");
              rs.result = -1;
              rs.status = STATUS_FAILED;
              return;
      }
    } else {
      actionContext = new ActionContext(&(RuleEngine::Instance().getConstants()), 
                        &session, &_command, &cp, &rs);
      session.getLongCallContext().setActionContext(actionContext);
      isnewevent = true;
    }

    switch(hc.getCommandId())
    {
    case HTTP_REQUEST:
        processRequest((HttpRequest&)hc, *actionContext, isnewevent ); break;
    case HTTP_RESPONSE:
        processResponse((HttpResponse&)hc, *actionContext, isnewevent ); break;
    case HTTP_DELIVERY:
        processDelivery((HttpResponse&)hc, *actionContext, isnewevent ); break;
    default:
        smsc_log_debug(logger, "HttpEventHandler: unknown command");
        rs.status = STATUS_FAILED;
        rs.result = -1;
    }
    return;
}

int HttpEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "request")   return EH_HTTP_REQUEST;
    if (str == "response")  return EH_HTTP_RESPONSE;
    if (str == "delivery")  return EH_HTTP_DELIVERY;
    return EH_UNKNOWN; 
}

}}}
