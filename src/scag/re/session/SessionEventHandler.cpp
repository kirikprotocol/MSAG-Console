#include "SessionEventHandler.h"
#include "scag/re/RuleEngine.h"

namespace scag { namespace re {

void SessionEventHandler::process(SCAGCommand& command, Session& session, RuleStatus& rs) { _process(session, rs); };

void SessionEventHandler::_process(Session& session, RuleStatus& rs)
{
    smsc_log_debug(logger, "Process EventHandler...");

    Infrastructure& istr = BillingManager::Instance().getInfrastructure();

    const Address& abonentAddr = session.getSessionKey().abonentAddr;

    int providerId = istr.GetProviderID(session.getRuleKey().serviceId);
    if (providerId == 0)
    {
        throw SCAGException("SessionEventHandler: Cannot find ProviderID for ServiceID=%d", session.getRuleKey().serviceId);
    }

    int operatorId = istr.GetOperatorID(abonentAddr), hi = propertyObject.HandlerId;
    if (operatorId == 0) 
    {
        throw SCAGException("SessionEventHandler: Cannot find OperatorID for %s abonent", abonentAddr.toString().c_str());
    }

    CommandProperty commandProperty(NULL, 0, abonentAddr, providerId, operatorId, session.getRuleKey().serviceId, 0, (CommandOperations)0);

    ActionContext context(RuleEngine::Instance().getConstants(), session, NULL, commandProperty, rs);

    try
    {
        RunActions(context);
    } catch (SCAGException& e)
    {
        smsc_log_error(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = STATUS_FAILED;
    } catch (std::exception& e)
    {
        smsc_log_error(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = STATUS_FAILED;
    }
    return;
}

int SessionEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "session_init")      return EH_SESSION_INIT;
    if (str == "session_destroy")   return EH_SESSION_DESTROY;
    return UNKNOWN; 
}

}}
