#ifndef __SCAG_SMPP_EVENT_HANDLER2__
#define __SCAG_SMPP_EVENT_HANDLER2__

#include "EventHandler2.h"

namespace scag2 {
namespace re {

class SmppEventHandler : public EventHandler
{
    // void ProcessModifyRespCommandOperation(Session& session, SmppCommand& command, CSmppDescriptor& smppDescriptor);
    // void ProcessModifyCommandOperation(Session& session, SmppCommand& command, CSmppDescriptor& smppDescriptor);

    // void ModifyOperationBeforeExecuting(Session& session, SmppCommand& command, CSmppDescriptor& smppDescriptor);
    // void ModifyOperationAfterExecuting(Session& session, SmppCommand& command, RuleStatus& ruleStatus, CSmppDescriptor& smppDescriptor);
public:
    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs); 
    
    virtual int StrToHandlerId(const std::string& str); //Method return HandlerId from string
};

}}


#endif
