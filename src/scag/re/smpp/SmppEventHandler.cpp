#include "SmppEventHandler.h"
#include "scag/re/smpp/SmppAdapter.h"
#include "scag/re/actions/ActionContext.h"
#include "scag/re/CommandBrige.h"

namespace scag { namespace re {
/*
enum SmppHandlerType
{
    RECEIPT = 100
};
  */

using namespace scag::re::smpp;




RuleStatus SmppEventHandler::process(SCAGCommand& command, Session& session)
{
    Hash<Property> _constants;
    RuleStatus rs;

    smsc::util::config::Config config;

    Statistics& _statistics = Statistics::Instance();

    
    
    SmppCommand * smppcommand = dynamic_cast<SmppCommand *>(&command);
    if (!smppcommand) throw SCAGException("SmppEventHandler: command is not 'smpp-type'");

    SmppCommandAdapter _command(*smppcommand);

    /////////////////////////////////////////

    SACC_TRAFFIC_INFO_EVENT_t ev;

    //_SmppCommand * cmd = smppcommand->operator ->();
    
    CommandBrige::makeTrafficEvent(*smppcommand, (int)propertyObject.HandlerId, session.getPrimaryKey(), ev);

    _statistics.registerSaccEvent(ev);



    /////////////////////////////////////////
    
   

    if (!session.startOperation(command))
    {
        //TODO: направить отлуп в стейт-машину
    }

    ActionContext context(_constants, session, _command,_statistics, command.getServiceId(), CommandBrige::getAbonentAddr(*smppcommand));

    smsc_log_debug(logger, "Process EventHandler...");

    std::list<Action *>::const_iterator it;

    //TODO: Fill default rs fields
    rs.status = true;
    context.setRuleStatus(rs);

    for (it = actions.begin(); it!=actions.end(); ++it)
    {
        if (!(*it)->run(context)) break;
    }

    rs = context.getRuleStatus();
    session.endOperation(rs);
    return rs;
}

int SmppEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "submit_sm")             return EventHandlerType::EH_SUBMIT_SM;
    if (str == "submit_sm_resp")        return EventHandlerType::EH_SUBMIT_SM_RESP;
    if (str == "deliver_sm")            return EventHandlerType::EH_DELIVER_SM;
    if (str == "deliver_sm_resp")       return EventHandlerType::EH_DELIVER_SM_RESP;
    if (str == "receipt")               return EventHandlerType::EH_RECEIPT;
    return UNKNOWN; 
}

}}
