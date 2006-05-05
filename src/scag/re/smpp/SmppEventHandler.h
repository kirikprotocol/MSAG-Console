#ifndef __SCAG_SMPP_EVENT_HANDLER__
#define __SCAG_SMPP_EVENT_HANDLER__


#include "scag/re/EventHandler.h"

namespace scag { namespace re {

class SmppEventHandler : public EventHandler
{
    void StartOperation(Session& session, SmppCommand& command, CSmppDiscriptor& smppDiscriptor);
    void EndOperation(Session& session, SmppCommand& command, RuleStatus& ruleStatus, CSmppDiscriptor& smppDiscriptor);
public:
    virtual RuleStatus process(SCAGCommand& command, Session& session); 
    
    virtual int StrToHandlerId(const std::string& str); //Method return HandlerId from string
};

}}


#endif
