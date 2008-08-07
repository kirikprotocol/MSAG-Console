#ifndef __SCAG_SMPP_EVENT_HANDLER2__
#define __SCAG_SMPP_EVENT_HANDLER2__


#include "scag/re/EventHandler2.h"

namespace scag2 {
namespace re {

class SmppEventHandler : public EventHandler
{
    void ProcessModifyRespCommandOperation(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor);
    void ProcessModifyCommandOperation(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor);

    void ModifyOperationBeforeExecuting(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor);
    void ModifyOperationAfterExecuting(Session& session, SmppCommand& command, RuleStatus& ruleStatus, CSmppDiscriptor& smppDiscriptor);
public:
    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs); 
    
    virtual int StrToHandlerId(const std::string& str); //Method return HandlerId from string
};

}}


#endif
