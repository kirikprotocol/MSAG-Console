#ifndef __SCAG_HTTP_EVENT_HANDLER__
#define __SCAG_HTTP_EVENT_HANDLER__

#include <scag/transport/http/HttpCommand.h>
#include "scag/re/EventHandler.h"

namespace scag { namespace re { namespace http { 

using namespace scag::transport::http;

class HttpEventHandler : public EventHandler
{
    RuleStatus processRequest(HttpRequest& command, Session& session, CommandProperty& commandProperty);
    RuleStatus processResponse(HttpResponse& command, Session& session, CommandProperty& commandProperty);
    RuleStatus processDelivery(HttpResponse& command, Session& session, CommandProperty& commandProperty);
public:
    virtual RuleStatus process(SCAGCommand& command, Session& session); 
    //Method return HandlerId from hander type string
    virtual int StrToHandlerId(const std::string& str);
};

}}}

#endif
