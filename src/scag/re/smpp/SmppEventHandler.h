#ifndef __SCAG_SMPP_EVENT_HANDLER__
#define __SCAG_SMPP_EVENT_HANDLER__


#include "scag/re/EventHandler.h"

namespace scag { namespace re {

class SmppEventHandler : public EventHandler
{
    void ProcessModifyRespCommandOperation(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor);
    void ProcessModifyCommandOperation(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor);

    void ModifyOperationBeforeExecuting(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor);
    void ModifyOperationAfterExecuting(Session& session, SmppCommand& command, RuleStatus& ruleStatus, CSmppDiscriptor& smppDiscriptor);
public:
    virtual RuleStatus process(SCAGCommand& command, Session& session, LongCallContext& longCallContext); 
    
    virtual int StrToHandlerId(const std::string& str); //Method return HandlerId from string
};

}}


#endif
