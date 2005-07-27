#ifndef __SCAG_SMPP_EVENT_HANDLER__
#define __SCAG_SMPP_EVENT_HANDLER__


#include "scag/re/EventHandler.h"

namespace scag { namespace re {


class SmppEventHandler : public EventHandler
{
protected: 
    virtual int StrToHandlerId(const std::string& str);
public:
    virtual void init(const SectionParams& params); 
    virtual RuleStatus process(SCAGCommand& command); 
};

}}


#endif
