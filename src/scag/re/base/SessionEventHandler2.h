#ifndef __SCAG_SESSION_EVENT_HANDLER2__
#define __SCAG_SESSION_EVENT_HANDLER2__


#include "EventHandler2.h"

namespace scag2 {
namespace re {

class SessionEventHandler : public EventHandler
{
public:
    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs, CommandProperty& cp, util::HRTiming* hrt = 0 );
    void _process(Session& session, RuleStatus& rs, const RuleKey& key);
    
    virtual int StrToHandlerId(const std::string& str); //Method return HandlerId from string
};

}}


#endif
