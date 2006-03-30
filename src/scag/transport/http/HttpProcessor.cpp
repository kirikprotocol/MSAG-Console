#include <string>

#include <scag/re/RuleEngine.h>
#include <scag/sessions/SessionManager.h>
#include <scag/stat/Statistics.h>

#include "HttpCommand.h"
#include "HttpProcessor.h"
#include "HttpRouter.h"

namespace scag { namespace transport { namespace http {

using namespace scag::sessions;
using namespace scag::re;

//    Logger *httpLogger = NULL;
    
class HttpProcessorImpl : public HttpProcessor
{
    HttpRouterImpl router;
public:
    virtual bool processRequest(HttpRequest& request)
    {
        SessionManager& smgr = SessionManager::Instance();
        CSessionKey sk = {request.getUSR(), request.getAbonent().c_str()};
        SessionPtr se = smgr.getSession(sk);

        if(!se.Get())
            return false;

        try{
            HttpRoute r = router.findRoute(request.getAbonent(), request.getSite());
            request.setServiceId(r.service_id);
        }
        catch(RouteNotFoundException& e)
        {
            return false;
        }

        RuleEngine& re = RuleEngine::Instance();
        re.process(request, *se.Get());

        return true;
    }

    virtual bool processResponse(HttpResponse& response)
    {
        return true;
    }

    virtual void statusResponse(const HttpResponse& response, bool delivered = true)
    {
    }
        
    virtual ~HttpProcessorImpl() {}
        
    HttpProcessorImpl() {}

    void init(std::string& cfg)
    {
        router.init(cfg);
    }
};

}}}
