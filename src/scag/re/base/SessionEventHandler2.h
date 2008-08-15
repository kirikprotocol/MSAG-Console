#ifndef __SCAG_SESSION_EVENT_HANDLER2__
#define __SCAG_SESSION_EVENT_HANDLER2__


#include "EventHandler2.h"

namespace scag2 {
namespace re {

class SessionEventHandler : public EventHandler
{
public:
    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs);
    void _process(Session& session, RuleStatus& rs);
    
    virtual int StrToHandlerId(const std::string& str); //Method return HandlerId from string
};

}}


#endif
