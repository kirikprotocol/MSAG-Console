#ifndef __SCAG_HTTP_EVENT_HANDLER2__
#define __SCAG_HTTP_EVENT_HANDLER2__

#include "scag/transport/http/base/HttpCommand2.h"
#include "EventHandler2.h"

namespace scag2 {
namespace re {
namespace http { 

class HttpEventHandler : public EventHandler
{
    void processRequest(transport::http::HttpRequest& command, ActionContext& context);
    void processResponse(transport::http::HttpResponse& command, ActionContext& context);
    void processDelivery(transport::http::HttpResponse& command, ActionContext& context);
public:
    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs);
    //Method return HandlerId from hander type string
    virtual int StrToHandlerId(const std::string& str);
};

}}}

#endif
