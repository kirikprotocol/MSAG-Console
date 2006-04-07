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
using namespace scag::stat::Counters;

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

        void registerEvent(int event, HttpCommand& cmd, RuleStatus& rs);
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

    RuleStatus rs;

    try{
        r = router.findRoute(request.getAbonent(), request.getSite(), request.getSitePath(), request.getSitePort());
        http_log_debug("httproute found route_id=%s, service_id=%d", r.id.c_str(), r.service_id);
        request.setServiceId(r.service_id);
        request.setRouteId(r.id);
        request.setProviderId(r.provider_id);

        CSessionKey sk = {request.getUSR(), request.getAbonent().c_str()};
        SessionPtr se = SessionManager::Instance().getSession(sk);

        if(se.Get())
        {
            rs = RuleEngine::Instance().process(request, *se.Get());

            if(rs.result >= 0)
            {
                registerEvent(httpRequest, request, rs);
                return true;
            }

        } else
            http_log_debug("session not found for abonent=%s, USR=%d", request.getAbonent().c_str(), request.getUSR());
    }
    catch(RouteNotFoundException& e)
    {
        http_log_debug("httproute not found");
    }

    registerEvent(httpRequestRejected, request, rs);
    return false;
}

bool HttpProcessorImpl::processResponse(HttpResponse& response)
{
    http_log_debug("Got http_response command abonent=%s, USR=%d, route_id=%s, service_id=%s", response.getAbonent().c_str(), response.getUSR(), response.getRouteId().c_str(), response.getServiceId());

    CSessionKey sk = {response.getUSR(), response.getAbonent().c_str()};
    SessionPtr se = SessionManager::Instance().getSession(sk);

    RuleStatus rs;

    if(se.Get())
    {
        rs = RuleEngine::Instance().process(response, *se.Get());
        if(rs.result >= 0)
        {
            registerEvent(httpRequest, response, rs);
            return true;
        }
    } else
        http_log_debug("http_response session not found abonent=%s, USR=%d", response.getAbonent().c_str(), response.getUSR());

    registerEvent(httpRequestRejected, response, rs);

    return false;
}

void HttpProcessorImpl::statusResponse(HttpResponse& response, bool delivered)
{
    http_log_debug("Got http_status_response command abonent=%s, USR=%d, route_id=%s, service_id=%s, delivered=%d",
             response.getAbonent().c_str(), response.getUSR(), response.getRouteId().c_str(), response.getServiceId(), delivered);

    CSessionKey sk = {response.getUSR(), response.getAbonent().c_str()};
    SessionPtr se = SessionManager::Instance().getSession(sk);

    RuleStatus rs;
    if(se.Get())
    {
        response.setCommandId(HTTP_DELIVERY);
        rs = RuleEngine::Instance().process(response, *se.Get());
        if(rs.result > 0)
        {
            registerEvent(httpRequest, response, rs);
        }
    }
    else
        http_log_debug("http_status_response session not found abonent=%s, USR=%d", response.getAbonent().c_str(), response.getUSR());

    registerEvent(httpRequestRejected, response, rs);
}
        
void HttpProcessorImpl::init(const std::string& cfg)
{
    router.init(cfg);
}

void HttpProcessorImpl::ReloadRoutes()
{
    router.ReloadRoutes();
}

void HttpProcessorImpl::registerEvent(int event, HttpCommand& cmd, RuleStatus& rs)
{
//TODO: Add http event registration
    Statistics::Instance().registerEvent(HttpStatEvent(event, cmd.getRouteId(), cmd.getServiceId(), cmd.getProviderId(), rs.result));
}

}}}
