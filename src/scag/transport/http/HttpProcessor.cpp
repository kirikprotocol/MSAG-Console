#include <string>

#include <scag/util/singleton/Singleton.h>
#include <scag/re/RuleEngine.h>
#include <scag/sessions/SessionManager.h>
#include <scag/stat/Statistics.h>

#include "HttpCommand.h"
#include "HttpProcessor.h"
#include "HttpRouter.h"

#include "logger/Logger.h"

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
        smsc::logger::Logger* logger;
        HttpRouterImpl router;

        void registerEvent(int event, HttpRequest& cmd);
        void registerEvent(int event, HttpResponse& cmd);
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

    smsc_log_debug( logger, "Got http_request command host=%s:%d, path=%s, abonent=%s, USR=%d", request.getSite().c_str(), request.getSitePort(), request.getSiteFull().c_str(), request.getAbonent().c_str(), request.getUSR());

    RuleStatus rs;
    SessionPtr se;

    try{
        r = router.findRoute(request.getAbonent(), request.getSite(), request.getSitePath(), request.getSitePort());
        smsc_log_debug( logger, "httproute found route_id=%s, service_id=%d", r.id.c_str(), r.service_id);
        request.setServiceId(r.service_id);
        request.setRouteId(r.id);
        request.setProviderId(r.provider_id);

        CSessionKey sk = {request.getUSR(), Address(request.getAbonent().c_str())};
        if(!request.getUSR())
        {
            se = SessionManager::Instance().newSession(sk);

            if(se.Get())
            {
                PendingOperation pendingOperation;
                pendingOperation.type = CO_HTTP_DELIVERY;
                pendingOperation.validityTime = time(NULL) + SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;
                se.Get()->addPendingOperation(pendingOperation);
                SessionManager::Instance().releaseSession(se);
            }

            request.setUSR(sk.USR);
        }
        se = SessionManager::Instance().getSession(sk);

        if(se.Get())
        {
            rs = RuleEngine::Instance().process(request, *se.Get());

            if(rs.result >= 0)
            {
                registerEvent(httpRequest, request);
                SessionManager::Instance().releaseSession(se);
                return true;
            }
        } else
            smsc_log_error( logger, "session not found for abonent=%s, USR=%d", request.getAbonent().c_str(), request.getUSR());
    }
    catch(RouteNotFoundException& e)
    {
        smsc_log_warn(logger, "route not found for abonent:%s, site:%s:%d%s", request.getAbonent().c_str(), request.getSite().c_str(), request.getSitePort(), request.getSitePath().c_str());
    }
    catch(Exception& e)
    {
        smsc_log_error(logger, "error processing request. %s", e.what());
    }

    if(se.Get())
        SessionManager::Instance().releaseSession(se);

    registerEvent(httpRequestRejected, request);

    return false;
}

bool HttpProcessorImpl::processResponse(HttpResponse& response)
{
    smsc_log_debug( logger, "Got http_response command abonent=%s, USR=%d, route_id=%s, service_id=%d", response.getAbonent().c_str(), response.getUSR(), response.getRouteId().c_str(), response.getServiceId());

    SessionPtr se;
    try{
        CSessionKey sk = {response.getUSR(), Address(response.getAbonent().c_str())};
        se = SessionManager::Instance().getSession(sk);
        RuleStatus rs;

        if(se.Get())
        {
            rs = RuleEngine::Instance().process(response, *se.Get());
            if(rs.result >= 0)
            {
                registerEvent(httpResponse, response);
                SessionManager::Instance().releaseSession(se);
                return true;
            }
        } else
            smsc_log_error( logger, "http_response session not found abonent=%s, USR=%d", response.getAbonent().c_str(), response.getUSR());
    }
    catch(Exception& e)
    {
        smsc_log_error( logger, "http_response error processing abonent=%s, USR=%d. %s", response.getAbonent().c_str(), response.getUSR(), e.what());
    }
    catch(...)
    {
        smsc_log_error( logger, "http_response error processing abonent=%s, USR=%d.", response.getAbonent().c_str(), response.getUSR());
    }

    if(se.Get())
        SessionManager::Instance().releaseSession(se);

    registerEvent(httpResponseRejected, response);

    return false;
}

void HttpProcessorImpl::statusResponse(HttpResponse& response, bool delivered)
{
    smsc_log_debug(logger, "Got http_status_response command abonent=%s, USR=%d, route_id=%s, service_id=%d, delivered=%d",
             response.getAbonent().c_str(), response.getUSR(), response.getRouteId().c_str(), response.getServiceId(), delivered);

    SessionPtr se;
    try{
        CSessionKey sk = {response.getUSR(), Address(response.getAbonent().c_str())};
        se = SessionManager::Instance().getSession(sk);

        RuleStatus rs;
        if(se.Get())
        {
            response.setCommandId(HTTP_DELIVERY);
            rs = RuleEngine::Instance().process(response, *se.Get());

            if(rs.result > 0)
            {
                registerEvent(httpDelivered, response);
                SessionManager::Instance().releaseSession(se);
                return;
            }
        }
        else
            smsc_log_error( logger, "http_status_response session not found abonent=%s, USR=%d", response.getAbonent().c_str(), response.getUSR());
    }
    catch(Exception& e)
    {
        smsc_log_error( logger, "http_status_response error processing abonent=%s, USR=%d. %s", response.getAbonent().c_str(), response.getUSR(), e.what());
    }
    catch(...)
    {
        smsc_log_error( logger, "http_status_response error processing abonent=%s, USR=%d.", response.getAbonent().c_str(), response.getUSR());
    }

    if(se.Get())
        SessionManager::Instance().releaseSession(se);

    registerEvent(httpFailed, response);
}
        
void HttpProcessorImpl::init(const std::string& cfg)
{
    logger = Logger::getInstance("httpProc");
    router.init(cfg + "/http_routes.xml");
}

void HttpProcessorImpl::ReloadRoutes()
{
    router.ReloadRoutes();
}

void HttpProcessorImpl::registerEvent(int event, HttpRequest& cmd)
{
    char buf[15];
    std::string s = cmd.getSite();
    if(cmd.getSitePort() != 80)
    {
        s += ":";
        sprintf(buf, "%d", cmd.getSitePort());
        s += buf;
    }
    Statistics::Instance().registerEvent(HttpStatEvent(event, cmd.getRouteId(), cmd.getServiceId(), cmd.getProviderId(), s, cmd.getSitePath(), 0));
}

void HttpProcessorImpl::registerEvent(int event, HttpResponse& cmd)
{
    Statistics::Instance().registerEvent(HttpStatEvent(event, cmd.getRouteId(), cmd.getServiceId(), cmd.getProviderId(), "", "", cmd.getStatus()));
}

}}}
