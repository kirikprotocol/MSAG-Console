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
        PlacementArray defInAddrPlaces;
        PlacementArray defOutAddrPlaces;
        PlacementArray defInUSRPlaces;
        PlacementArray defOutUSRPlaces;

        smsc::logger::Logger* logger;
        HttpRouterImpl router;
        bool addrInURL, USRInURL;

//        bool findPlace(std::string& rs, const PlacementArray& places, HttpRequest& request);
        bool findPlace(const char* wh, std::string& rs, const PlacementArray& places, HttpRequest& request);
        bool findUSR(HttpRequest& request, const PlacementArray& places);
        bool findAddress(HttpRequest& request, const PlacementArray& places);
        bool setUSR(HttpRequest& request, const PlacementArray& places);
        bool setAbonent(HttpRequest& request, const PlacementArray& places);
        bool setPlaces(const std::string& rs, const PlacementArray& places, HttpRequest& request);
        bool parsePath(bool addr, const std::string &path, HttpRequest& cx);
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

bool HttpProcessorImpl::parsePath(bool addr, const std::string &path, HttpRequest& cx)
{
  const char    *pos = path.c_str(), *mid, *end;
  std::string   str;
  unsigned int len;
   
  smsc_log_debug(logger, "IN ADDR=%d", addr);
  end = strchr(pos, '/');
  if (!end)
  {
    smsc_log_debug(logger, "if (!end)1");
    return false;
  }

    if(addr)
    {
        if (end == pos)
        {
            pos++;
            end = strchr(pos, '/');
            if (!end)
            {
                smsc_log_debug(logger, "if (!end)2");
              return false;
                }
        }

        mid = pos;
        while (mid <= end && (isalnum(*mid) || *mid == '+' || *mid == '.'))
            mid++;

        len = mid - pos;
        if (!(mid <= end && 1 <= len && len <= 20))
        {
            smsc_log_debug(logger, "if (!(mid <= end && 1 <= len && len <= 20))");
            return false;
        }

        str.assign(pos, len);
//        Address addr(str.c_str());
        cx.setAbonent(str);

        if(*mid == '_')
        {
            mid++;
            len = end - mid;
            str.assign(mid, len);
  
            pos = str.c_str();
            while (isdigit(*pos))
                pos++;
            if (*pos || !(1 <= len && len <= 5))
              return false;
  
            len = atoi(str.c_str());
            if (len > USHRT_MAX | !len)
            {
                smsc_log_debug(logger, "if (len > USHRT_MAX | !len)");
                return false;
            }
  
            cx.setUSR(len);
        }
    }
 
  pos = end + 1;
  end = strchr(pos, '/');
  if (!end)
  {
    smsc_log_debug(logger, "if (!end)3");
    return false;
    }

  mid = strchr(pos, ':');
  if (mid && (mid < end)) {
    str.assign(pos, mid - pos);
    cx.setSite(str);
    mid++;
    str.assign(mid, end - mid);
    cx.setSitePort(atoi(str.c_str()));
  } else {
    str.assign(pos, end - pos);
    cx.setSite(str);
  }

  pos = end;
  if (*pos)
  {
    str.assign(pos);
    end = strrchr(pos, '/');
    str.assign(pos, end + 1 - pos);
    cx.setSitePath(str);
    str.assign(end + 1);
    cx.setSiteFileName(str);
  }

  return true;
}

bool HttpProcessorImpl::findPlace(const char* wh, std::string& rs, const PlacementArray& places, HttpRequest& request)
{
    for(int i = 0; i < places.Count(); i++)
    {
        switch(places[i].type)
        {
            case PlacementType::PARAM:
            {
                const std::string& s = request.getQueryParameter(places[i].name);
                if(!s.length()) break;
                rs = s;
                smsc_log_debug(logger, "%s FOUND IN PARAM: %s=%s", wh, places[i].name.c_str(), rs.c_str());
                return true;
            }
            case PlacementType::COOKIE:
            {
                Cookie *c = request.getCookie(places[i].name);
                if(c == NULL || !c->value.length()) break;

                rs = c->value;
                smsc_log_debug(logger, "%s FOUND IN COOKIE: %s=%s", wh, places[i].name.c_str(), rs.c_str());
                return true;
            }
            case PlacementType::HEADER:
            {
                const std::string& s = request.getHeaderField(places[i].name);
                if(!s.length()) break;
                rs = s;
                smsc_log_debug(logger, " %s FOUND IN HEADER: %s=%s", wh, places[i].name.c_str(), rs.c_str());
                return true;;
            }
            case PlacementType::URL:
            {
                if(!rs.length()) break;

                smsc_log_debug(logger, " %s FOUND IN URL: %s=%s", wh, places[i].name.c_str(), rs.c_str());
                return true;
            }
        }
    }
    smsc_log_debug( logger, "%s NOT FOUND", wh);
    return false;
}

bool HttpProcessorImpl::findAddress(HttpRequest& request, const PlacementArray& places)
{
    std::string s = request.getAbonent();
    if(findPlace("ADDRESS", s, places, request))
    {
        request.setAbonent(s);
        return true;
    }
    return false;
}

bool HttpProcessorImpl::findUSR(HttpRequest& request, const PlacementArray& places)
{
    std::string s;

    if(request.getUSR())
    {
        char buf[20];
        buf[19] = 0;
        s = lltostr(request.getUSR(), buf + 19);
    }

    uint16_t i;

    if(findPlace("USR", s, places, request) && (i = atoi(s.c_str())))
    {
        request.setUSR(i);
        return true;
    }
    return false;
}

bool HttpProcessorImpl::setUSR(HttpRequest& request, const PlacementArray& places)
{
    std::string s;
    uint16_t i;

    char buf[20];
    buf[19] = 0;
    return setPlaces(lltostr(request.getUSR(), buf + 19), places, request);
}

bool HttpProcessorImpl::setAbonent(HttpRequest& request, const PlacementArray& places)
{
    return setPlaces(request.getAbonent(), places, request);
}

bool HttpProcessorImpl::setPlaces(const std::string& rs, const PlacementArray& places, HttpRequest& request)
{
    bool in_url = false;
    std::string s;

    for(int i = 0; i < places.Count(); i++)
    {
        switch(places[i].type)
        {
            case PlacementType::PARAM:
            {
                request.setQueryParameter(places[i].name, rs);
                break;
            }
            case PlacementType::COOKIE:
            {
                Cookie *c = request.setCookie(places[i].name, rs);
                break;
            }
            case PlacementType::HEADER:
                request.setHeaderField(places[i].name, rs);
                break;
            case PlacementType::URL:
                in_url = true;
                break;
        }
    }
    return in_url;
}

bool HttpProcessorImpl::processRequest(HttpRequest& request)
{
    HttpRoute r;

    RuleStatus rs;
    SessionPtr se;

    try{
        if(!parsePath(addrInURL, request.getSitePath(), request))
        {
            registerEvent(scag::stat::events::http::REQUEST_FAILED, request);
            smsc_log_debug( logger, "http_request path parse error %s", request.getSitePath().c_str());
            return false;
        }

        if(!findAddress(request, defInAddrPlaces))
        {
            registerEvent(scag::stat::events::http::REQUEST_FAILED, request);
            smsc_log_debug( logger, "http_request address not found %s", request.getSitePath().c_str());
            return false;
        }

        smsc_log_debug( logger, "Got http_request command host=%s:%d, path=%s, filename=%s, abonent=%s, USR=%d", request.getSite().c_str(), request.getSitePort(), request.getSitePath().c_str(), request.getSiteFileName().c_str(), request.getAbonent().c_str(), request.getUSR());

        smsc_log_debug(logger, "SERIALIZED REQUEST BEFORE PROCESSING: %s", request.serialize().c_str());
            
        r = router.findRoute(request.getAbonent(), request.getSite(), request.getSitePath() + request.getSiteFileName(), request.getSitePort());
        smsc_log_debug( logger, "httproute found route_id=%s, service_id=%d", r.id.c_str(), r.service_id);
        request.setServiceId(r.service_id);
        request.setRouteId(r.id);
        request.setProviderId(r.provider_id);

        findUSR(request, r.inUSRPlace.Count() ? r.inUSRPlace : defInUSRPlaces);

        request.setAddress(r.addressPrefix + request.getAbonent());

        CSessionKey sk = {request.getUSR(), request.getAddress().c_str()};
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
                registerEvent(scag::stat::events::http::REQUEST_OK, request);
                SessionManager::Instance().releaseSession(se);

                bool a = setAbonent(request, r.outAddressPlace.Count() ? r.outAddressPlace : defOutAddrPlaces);
                bool u = setUSR(request, r.outUSRPlace.Count() ? r.outUSRPlace : defOutUSRPlaces);
                std::string h;

                if(a || u)
                {
                    if(request.getSiteFileName().length())
                        h += '/';
                    if(a)
                        h += request.getAbonent();
                    if(u)
                    {
                        char buf[20];
                        buf[19] = 0;
                        h += '_';
                        h += lltostr(request.getUSR(), buf + 19);
                    }
                    if(h.length())
                        request.setSiteFileName(request.getSiteFileName() + h);
                }

                smsc_log_debug(logger, "SERIALIZED REQUEST AFTER PROCESSING: %s", request.serialize().c_str());
                return true;
            }
        } else
            smsc_log_error( logger, "session not found for addr=%s, USR=%d", request.getAddress().c_str(), request.getUSR());
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

    registerEvent(scag::stat::events::http::REQUEST_FAILED, request);

    return false;
}

bool HttpProcessorImpl::processResponse(HttpResponse& response)
{
    smsc_log_debug( logger, "Got http_response command abonent=%s, USR=%d, route_id=%s, service_id=%d", response.getAbonent().c_str(), response.getUSR(), response.getRouteId().c_str(), response.getServiceId());

    SessionPtr se;
    try{
        CSessionKey sk = {response.getUSR(), response.getAddress().c_str()};
        se = SessionManager::Instance().getSession(sk);
        RuleStatus rs;

        if(se.Get())
        {
            rs = RuleEngine::Instance().process(response, *se.Get());
            if(rs.result >= 0)
            {
                registerEvent(scag::stat::events::http::RESPONSE_OK, response);
                SessionManager::Instance().releaseSession(se);
                return true;
            }
        } else
            smsc_log_error( logger, "http_response session not found abonent=%s, USR=%d", response.getAddress().c_str(), response.getUSR());
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

    registerEvent(scag::stat::events::http::RESPONSE_FAILED, response);

    return false;
}

void HttpProcessorImpl::statusResponse(HttpResponse& response, bool delivered)
{
    smsc_log_debug(logger, "Got http_status_response command abonent=%s, USR=%d, route_id=%s, service_id=%d, delivered=%d",
             response.getAbonent().c_str(), response.getUSR(), response.getRouteId().c_str(), response.getServiceId(), delivered);

    SessionPtr se;
    try{
        CSessionKey sk = {response.getUSR(), response.getAddress().c_str()};
        se = SessionManager::Instance().getSession(sk);

        RuleStatus rs;
        if(se.Get())
        {
            response.setCommandId(HTTP_DELIVERY);
            rs = RuleEngine::Instance().process(response, *se.Get());

            if(rs.result > 0 && delivered)
            {
                registerEvent(scag::stat::events::http::DELIVERED, response);
                SessionManager::Instance().releaseSession(se);
                return;
            }
        }
        else
            smsc_log_error( logger, "http_status_response session not found abonent=%s, USR=%d", response.getAddress().c_str(), response.getUSR());
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

    registerEvent(scag::stat::events::http::FAILED, response);
}
        
void HttpProcessorImpl::init(const std::string& cfg)
{
    logger = Logger::getInstance("httpProc");
    router.init(cfg + "/http_routes.xml");
    ReloadRoutes();
}

void HttpProcessorImpl::ReloadRoutes()
{
    router.ReloadRoutes();

    router.getDefaultInAddressPlacement(defInAddrPlaces);
    router.getDefaultInUSRPlacement(defInUSRPlaces);
    router.getDefaultOutAddressPlacement(defOutAddrPlaces);
    router.getDefaultOutUSRPlacement(defOutUSRPlaces);

    int i = 0;
    while(i < defInAddrPlaces.Count() && defInAddrPlaces[i].type != PlacementType::URL)
        i++;

    addrInURL = i < defInAddrPlaces.Count();

    i = 0;
    while(i < defInUSRPlaces.Count() && defInUSRPlaces[i].type != PlacementType::URL)
        i++;

    USRInURL = i < defInUSRPlaces.Count();
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
