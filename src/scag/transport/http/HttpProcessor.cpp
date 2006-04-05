#include <string>

#include <scag/util/singleton/Singleton.h>
#include <scag/re/RuleEngine.h>
#include <scag/sessions/SessionManager.h>
#include <scag/stat/Statistics.h>

#include "HttpCommand.h"
#include "HttpProcessor.h"
#include "HttpRouter.h"

#include "HttpLogger.h"

namespace scag { namespace transport { namespace http {

using namespace scag::util::singleton;
using namespace scag::sessions;
using namespace scag::re;

class HttpProcessorImpl : public HttpProcessor
{
    public:
        virtual bool processRequest(HttpRequest& request);
        virtual bool processResponse(HttpResponse& response);
        virtual void statusResponse(HttpResponse& response, bool delivered=true);
        virtual void ReloadRoutes();

        void init(const std::string& cfg);

        virtual ~HttpProcessorImpl() {}
    protected:
        HttpRouterImpl router;

        bool process(HttpCommand& cmd);
};

static bool  inited = false;
static Mutex initLock;

inline unsigned GetLongevity(HttpProcessor*) { return 5; }
typedef SingletonHolder<HttpProcessorImpl> SingleHP;

HttpProcessor& HttpProcessor::Instance()
{
    if (!inited) 
    {
        MutexGuard guard(initLock);
        if (!inited) 
            throw std::runtime_error("HttpProcessor not inited!");
    }
    return SingleHP::Instance();
}

void HttpProcessor::Init(const std::string& cfg)
{
    if (!inited)
    {
        MutexGuard guard(initLock);
        if(!inited) {
            HttpProcessorImpl& hp = SingleHP::Instance();
            hp.init(cfg);
            inited = true;
        }
    }
}

bool HttpProcessorImpl::processRequest(HttpRequest& request)
{
    HttpRoute r;

    http_log_debug("Got http_request command host=%s:%d, path=%s, abonent=%s", request.getSite().c_str(), request.getSitePath().c_str(), request.getAbonent().c_str());

    try{
        r = router.findRoute(request.getAbonent(), request.getSite(), request.getSitePath(), request.getSitePort());
        http_log_debug("httproute found route_id=%s, service_id=%d", r.id.c_str(), r.service_id);
        request.setServiceId(r.service_id);
        request.setRouteId(r.id);
    }
    catch(RouteNotFoundException& e)
    {
        http_log_debug("httproute not found");
        return false;
    }

    CSessionKey sk = {request.getUSR(), request.getAbonent().c_str()};
    SessionPtr se = SessionManager::Instance().getSession(sk);

    if(!se.Get())
    {
        http_log_debug("session not found for abonent=%s, USR=%d", request.getAbonent().c_str(), request.getUSR());
        return false;
    }
//TODO: Add register event
//    Statistics::Instance().registerEvent(HttpStatEvent(0, request.getRouteId(), r.service_id, r.provider_id));

    RuleStatus rs = RuleEngine::Instance().process(request, *se.Get());

//    Statistics::Instance().registerEvent(HttpStatEvent(0, request.getRouteId(), r.service_id, r.provider_id, rs.result));

    if(rs.result < 0)
        return false;

    return true;
}

bool HttpProcessorImpl::processResponse(HttpResponse& response)
{
    http_log_debug("Got http_response command abonent=%s, USR=%d, route_id=%s, service_id=%s", response.getAbonent().c_str(), response.getUSR(), response.getRouteId().c_str(), response.getServiceId());

    CSessionKey sk = {response.getUSR(), response.getAbonent().c_str()};
    SessionPtr se = SessionManager::Instance().getSession(sk);

    if(!se.Get())
    {
        http_log_debug("http_response session not found abonent=%s, USR=%d", response.getAbonent().c_str(), response.getUSR());
        return false;
    }

//    Statistics::Instance().registerEvent(HttpStatEvent(0, response.getRouteId(), "", -1));

    RuleStatus rs = RuleEngine::Instance().process(response, *se.Get());

//    Statistics::Instance().registerEvent(HttpStatEvent(0, response.getRouteId(), "", -1, rs.result));

    if(rs.result < 0)
        return false;

    return true;
}

void HttpProcessorImpl::statusResponse(HttpResponse& response, bool delivered)
{
    http_log_debug("Got http_status_response command abonent=%s, USR=%d, route_id=%s, service_id=%s, delivered=%d",
             response.getAbonent().c_str(), response.getUSR(), response.getRouteId().c_str(), response.getServiceId(), delivered);

    CSessionKey sk = {response.getUSR(), response.getAbonent().c_str()};
    SessionPtr se = SessionManager::Instance().getSession(sk);

    if(!se.Get())
    {
        http_log_debug("http_status_response session not found abonent=%s, USR=%d", response.getAbonent().c_str(), response.getUSR());
        return;
    }

//    Statistics::Instance().registerEvent(HttpStatEvent(0, response.getRouteId(), "", -1));

    response.setCommandId(HTTP_DELIVERY);

    RuleStatus rs = RuleEngine::Instance().process(response, *se.Get());
}
        
void HttpProcessorImpl::init(const std::string& cfg)
{
    router.init(cfg);
}
void HttpProcessorImpl::ReloadRoutes()
{
    router.ReloadRoutes();
}

}}}
