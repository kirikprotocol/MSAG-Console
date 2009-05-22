#include "SmppEventHandler2.h"
#include "SmppAdapter2.h"
#include "ActionContext2.h"
#include "CommandBridge.h"
#include "RuleEngine2.h"
#include "scag/sessions/base/Operation.h"
#include "scag/bill/base/BillingManager.h"
#include "scag/util/HRTimer.h"

namespace scag2 {
namespace re {

using namespace smpp;

void SmppEventHandler::process( SCAGCommand& command, Session& session, RuleStatus& rs, CommandProperty& cp, util::HRTiming* inhrt )
{
    // smsc_log_debug(logger, "Process EventHandler...");
    util::HRTiming hrt(inhrt);

    SmppCommand& smppcommand = static_cast<SmppCommand&>(command);
    SmppCommandAdapter _command(smppcommand);
/*
    bill::Infrastructure& istr = bill::BillingManager::Instance().getInfrastructure();

    const Address abonentAddr( session.sessionKey().toString().c_str() );
    // CSmppDescriptor smppDescriptor = CommandBridge::getSmppDescriptor(smppcommand);

    int providerId = istr.GetProviderID(command.getServiceId());
    if (providerId == 0) 
    {
        if ( smppcommand.isResp() ) session.closeCurrentOperation();
        throw SCAGException("SmppEventHandler: Cannot find ProviderID for ServiceID=%d", command.getServiceId());
    }

    int operatorId = istr.GetOperatorID(abonentAddr), hi = propertyObject.HandlerId;
    if (operatorId == 0) 
    {
        RegisterAlarmEvent(1, abonentAddr.toString(), CommandBridge::getProtocolForEvent(smppcommand), command.getServiceId(),
                           providerId, 0, 0, session.sessionPrimaryKey(),
                           (hi == EH_SUBMIT_SM)||(hi == EH_DELIVER_SM) ? 'I' : 'O');
        
        if ( smppcommand.isResp() ) session.closeCurrentOperation();
        throw SCAGException("SmppEventHandler: Cannot find OperatorID for %s abonent", abonentAddr.toString().c_str());
    }

    SMS& sms = CommandBridge::getSMS(smppcommand);
    int msgRef = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ? sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE):-1;

    //uint32_t commandStatus = (smppcommand->cmdid == DELIVERY_RESP || smppcommand->cmdid == SUBMIT_RESP || smppcommand->cmdid == DATASM_RESP) 
	//		    ? smppcommand->get_resp()->status : smppcommand->status;
    Property routeId;
    routeId.setStr(sms.getRouteId());
    CommandProperty commandProperty
        ( &command, smppcommand.get_status(), abonentAddr, providerId, operatorId,
          smppcommand.getServiceId(), msgRef,
          transport::CommandOperation(session.getCurrentOperation()->type()),
          routeId );
*/
    const bool newevent = ( !session.getLongCallContext().continueExec );
    
    ActionContext* actionContext = 0;
    if(session.getLongCallContext().continueExec) {
    	actionContext = session.getLongCallContext().getActionContext();
    	if (actionContext) {
	        actionContext->resetContext(&(RuleEngine::Instance().getConstants()), 
	                    				&session, &_command, &cp, &rs);
    	} else {
	        smsc_log_error(logger, "EventHandler cannot get actionContext to continue");
    	    rs.result = smsc::system::Status::SMDELIFERYFAILURE;
    	    rs.status = STATUS_FAILED;
    	    return;
    	}
    } else {
    	actionContext = new ActionContext(&(RuleEngine::Instance().getConstants()), 
                    					  &session, &_command, &cp, &rs);
    	session.getLongCallContext().setActionContext(actionContext);
    }
    hrt.mark("ev.prep");
    
    try {
        RunActions(*actionContext);
    } catch (SCAGException& e) {
        smsc_log_error(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = STATUS_FAILED;
    } catch (std::exception& e) {
        smsc_log_error(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = STATUS_FAILED;
    }
    hrt.mark("ev.run");

    int hi = propertyObject.HandlerId;
    if ( newevent ) {
        const std::string* kw = 
            ( session.getCurrentOperation() ?
              session.getCurrentOperation()->getKeywords() : 0 );
        RegisterTrafficEvent( cp,
                              session.sessionPrimaryKey(),
                              (hi == EH_SUBMIT_SM) || 
                              (hi == EH_DELIVER_SM) ||
                              (hi == EH_DATA_SM) ?
                              CommandBridge::getMessageBody(smppcommand) : "",
                              kw,
                              &hrt );
    }

    if ( smppcommand.get_status() > 0) {
        rs.result = smppcommand.get_status();
        rs.status = STATUS_FAILED;
        smsc_log_debug( logger, "Command status=%d(%x) overrides RE status", rs.result, rs.result );
    }
    hrt.mark("ev.post");
    return;
}


int SmppEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "submit_sm")             return EH_SUBMIT_SM;
    if (str == "submit_sm_resp")        return EH_SUBMIT_SM_RESP;
    if (str == "deliver_sm")            return EH_DELIVER_SM;
    if (str == "deliver_sm_resp")       return EH_DELIVER_SM_RESP;
    if (str == "receipt")               return EH_RECEIPT;

    if (str == "data_sm")               return EH_DATA_SM;
    if (str == "data_sm_resp")          return EH_DATA_SM_RESP;

    return UNKNOWN; 
}

}}
