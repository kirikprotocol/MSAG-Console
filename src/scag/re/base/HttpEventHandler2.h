#ifndef __SCAG_HTTP_EVENT_HANDLER2__
#define __SCAG_HTTP_EVENT_HANDLER2__

#include "scag/transport/http/base/HttpCommand2.h"
#include "EventHandler2.h"

namespace scag2 {
namespace re {
namespace http { 

class HttpEventHandler : public EventHandler
{
    void processRequest(transport::http::HttpRequest& command, ActionContext& context, bool isnewevent );
    void processResponse(transport::http::HttpResponse& command, ActionContext& context, bool isnewevent );
    void processDelivery(transport::http::HttpResponse& command, ActionContext& context, bool isnewevent );
    /// extends EventHandler::RunActions, adds traffic event registration if isnewevent is true
    void newEvent( ActionContext& ctx );
public:
    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs, util::HRTiming* hrt = 0 );
    //Method return HandlerId from hander type string
    virtual int StrToHandlerId(const std::string& str);
};

}}}

#endif
