#ifndef __SCAG_HTTP_EVENT_HANDLER__
#define __SCAG_HTTP_EVENT_HANDLER__

#include <scag/transport/http/HttpCommand.h>
#include "scag/re/EventHandler.h"

namespace scag { namespace re { namespace http { 

using namespace scag::transport::http;

class HttpEventHandler : public EventHandler
{
    void processRequest(HttpRequest& command, ActionContext& context);
    void processResponse(HttpResponse& command, ActionContext& context);
    void processDelivery(HttpResponse& command, ActionContext& context);
public:
    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs); 
    //Method return HandlerId from hander type string
    virtual int StrToHandlerId(const std::string& str);
};

}}}

#endif
