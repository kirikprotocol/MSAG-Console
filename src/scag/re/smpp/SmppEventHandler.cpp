#include "SmppEventHandler.h"
#include "scag/re/smpp/SmppAdapter.h"
#include "scag/re/actions/ActionContext.h"

//#include "scag/SAX2Print.hpp"
class st : public scag::stat::Statistics
{
public:
    virtual bool checkTraffic(std::string routeId, scag::stat::CheckTrafficPeriod period, int64_t value) 
    {
        return true;
    }
    virtual void registerEvent(const scag::stat::SmppStatEvent &si) {}
};


namespace scag { namespace re {

enum SmppHandlerType
{
    RECEIPT = 100
};


using namespace scag::re::smpp;

RuleStatus SmppEventHandler::process(SCAGCommand& command, Session& session)
{
    Hash<Property> _constants;
    RuleStatus rs;

    smsc::util::config::Config config;
    st _statistics;

    SmppCommand * smppcommand = dynamic_cast<SmppCommand *>(&command);
    if (!smppcommand) throw SCAGException("SmppEventHandler: command is not 'smpp-type'");

    SmppCommandAdapter _command(*smppcommand);
    
    session.startOperation(command);

    ActionContext context(_constants, session, _command,_statistics);

    smsc_log_debug(logger, "Process EventHandler...");

    std::list<Action *>::const_iterator it;

    //TODO: Fill default rs fields
    rs.status = true;
    context.SetRuleStatus(rs);

    for (it = actions.begin(); it!=actions.end(); ++it)
    {
        if (!(*it)->run(context)) break;
    }

    rs = context.getStatus();
    return rs;
}

int SmppEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "submit_sm")             return SUBMIT;
    if (str == "submit_sm_resp")        return SUBMIT_RESP;
    if (str == "deliver_sm")            return DELIVERY;
    if (str == "deliver_sm_resp")       return DELIVERY_RESP;
    if (str == "reciept")               return RECEIPT;
/*
    if (str == "FORWARD")               return FORWARD;
    if (str == "GENERIC_NACK")          return GENERIC_NACK;
    if (str == "QUERY")                 return QUERY;
    if (str == "QUERY_RESP")            return QUERY_RESP;
    if (str == "UNBIND")                return UNBIND;
    if (str == "UNBIND_RESP")           return UNBIND_RESP;
    if (str == "REPLACE")               return REPLACE;
    if (str == "REPLACE_RESP")          return REPLACE_RESP;
    if (str == "CANCEL")                return CANCEL;
    if (str == "CANCEL_RESP")           return CANCEL_RESP;
    if (str == "ENQUIRELINK")           return ENQUIRELINK;
    if (str == "ENQUIRELINK_RESP")      return ENQUIRELINK_RESP;
    if (str == "SUBMIT_MULTI_SM")       return SUBMIT_MULTI_SM;
    if (str == "SUBMIT_MULTI_SM_RESP")  return SUBMIT_MULTI_SM_RESP;
    if (str == "BIND_TRANSCEIVER")      return BIND_TRANSCEIVER;
    if (str == "BIND_RECIEVER_RESP")    return BIND_RECIEVER_RESP;
    if (str == "BIND_TRANSMITTER_RESP") return BIND_TRANSMITTER_RESP;
    if (str == "BIND_TRANCIEVER_RESP")  return BIND_TRANCIEVER_RESP;
*/
    return UNKNOWN; 
}

}}
