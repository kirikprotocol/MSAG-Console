#ifndef __SCAG_SMPP_EVENT_HANDLER__
#define __SCAG_SMPP_EVENT_HANDLER__


#include "scag/re/EventHandler.h"

namespace scag { namespace re {


class SmppEventHandler : public EventHandler
{
public:
    virtual RuleStatus process(SCAGCommand& command, Session& session); 
    //Method return HandlerId from hander type string
    virtual int StrToHandlerId(const std::string& str);
};

}}


#endif
