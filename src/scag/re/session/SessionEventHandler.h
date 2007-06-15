#ifndef __SCAG_SESSION_EVENT_HANDLER__
#define __SCAG_SESSION_EVENT_HANDLER__


#include "scag/re/EventHandler.h"

namespace scag { namespace re {

class SessionEventHandler : public EventHandler
{
public:
    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs);
    void _process(Session& session, RuleStatus& rs);
    
    virtual int StrToHandlerId(const std::string& str); //Method return HandlerId from string
};

}}


#endif
