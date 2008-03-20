#include <string>

#include <scag/util/singleton/Singleton.h>
#include <scag/re/RuleEngine.h>
#include <scag/sessions/SessionManager.h>
#include <scag/stat/Statistics.h>
#include "scag/lcm/LongCallManager.h"

#include "HttpCommand.h"
#include "HttpProcessor.h"
#include "HttpRouter.h"
#include "Managers.h"

#include "logger/Logger.h"

namespace scag { namespace transport { namespace http {

using namespace scag::util::singleton;
using namespace scag::sessions;
using namespace scag::re;

class HttpProcessorImpl : public HttpProcessor
{
    public:
        virtual int processRequest(HttpRequest& request);
        virtual int processResponse(HttpResponse& response);
        virtual int statusResponse(HttpResponse& response, bool delivered=true);
        virtual void ReloadRoutes();

        void init(const std::string& cfg);
        virtual ~HttpProcessorImpl() {}
    protected:
        PlacementKindArray defInPlaces;
        PlacementKindArray defOutPlaces;

        smsc::logger::Logger* logger;
        HttpRouterImpl router;
        std::string inURLFields;

//        bool findPlace(std::string& rs, const PlacementArray& places, HttpRequest& request);
        bool findPlace(const char* wh, std::string& rs, const PlacementArray& places, HttpRequest& request, std::string& url);
        bool findUSR(HttpRequest& request, const PlacementArray& places);
        const PlacementArray& getOutPlaces(const HttpRoute& r, uint32_t t);
        const PlacementArray& getInPlaces(const HttpRoute& r, uint32_t t);
        bool defInURL(uint32_t t);
        void setPlaces(const std::string& rs, const PlacementArray& places, HttpRequest& request, std::string& url);
        void setFields(HttpRequest& request, HttpRoute& r);
        bool parsePath(const std::string &path, HttpRequest& cx);
        void registerEvent(int event, HttpCommand& cmd, bool delivery = false);
        bool makeLongCall(HttpCommand& cmd, SessionPtr& se);
};

bool  HttpProcessor::inited = false;
Mutex HttpProcessor::initLock;

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

/*bool HttpProcessorImpl::parseURLFields(char* pos, char* end, HttpRequest& cx)
{
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
}*/

bool HttpProcessorImpl::parsePath(const std::string &path, HttpRequest& cx)
{
  const char    *pos = path.c_str(), *mid, *end;
  std::string   str, rs;
  unsigned int len, i;

  if(!strncmp(pos, "http://", 7))
  {
    pos += 7;
    pos = strchr(pos, '/');
    if(!pos)
    {
        smsc_log_debug(logger, "No slash after host in absoulte path");
        return false;
    }
  }
  end = strchr(pos, '/');
  if (!end)
  {
    smsc_log_debug(logger, "No slash in path 1");
    return false;
  }

    if (end == pos)
    {
        pos++;
        end = strchr(pos, '/');
        if (!end)
        {
          smsc_log_debug(logger, "No slash in path 2");
          return false;
        }
    }

    inURLFields.assign(pos, end);
    
  i = 0;
  if(findPlace("ROUTE_ID", rs, defInPlaces[PlacementKind::ROUTE_ID], cx, inURLFields))
    i = atoi(rs.c_str());
    
  if(i > 0)    
    cx.setRouteId(i);
  else
  {      
      if(findPlace("SERVICE_ID", rs, defInPlaces[PlacementKind::SERVICE_ID], cx, inURLFields))
          i = atoi(rs.c_str());
      if(i > 0) cx.setServiceId(i);
  }
  
  if(findPlace("ADDR", rs, defInPlaces[PlacementKind::ADDR], cx, inURLFields))
      cx.setAbonent(rs);
 
  if(cx.getRouteId() <= 0 && cx.getServiceId() <= 0)
  {
      pos = end + 1;
      end = strchr(pos, '/');
      if (!end)
      {
        smsc_log_debug(logger, "No slash in path 1");
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
  }

  pos = end;
  if (*pos)
  {
//    str.assign(pos);
    end = strrchr(pos, '/');
    str.assign(pos, end + 1 - pos);
    cx.setSitePath(str);
    str.assign(end + 1);
    cx.setSiteFileName(str);
  }

  return true;
}

bool HttpProcessorImpl::findPlace(const char* wh, std::string& rs, const PlacementArray& places, HttpRequest& request, std::string& url)
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
                const char *p = url.c_str();
                while((p = strstr(p, places[i].name.c_str())) && p > url.c_str() && p[-1] >= 'A') p++;
                
                if(!p) break;
                
                p = p + places[i].name.length();
                rs.clear();
                while(*p && (isdigit(*p) || *p == '+' || *p == '.'))
                    rs += *p++;
                    
                if(!rs.length()) break;

                smsc_log_debug(logger, " %s FOUND IN URL: %s=%s", wh, places[i].name.c_str(), rs.c_str());
                return true;
            }
        }
    }
    smsc_log_debug( logger, "%s NOT FOUND", wh);
    return false;
}

bool HttpProcessorImpl::findUSR(HttpRequest& request, const PlacementArray& places)
{
    std::string s;
    uint16_t i;

    if(findPlace("USR", s, places, request, inURLFields) && (i = atoi(s.c_str())))
    {
        request.setUSR(i);
        return true;
    }
    return false;
}

const PlacementArray& HttpProcessorImpl::getOutPlaces(const HttpRoute& r, uint32_t t)
{
    return r.outPlace[t].Count() ? r.outPlace[t] : defOutPlaces[t];
}

const PlacementArray& HttpProcessorImpl::getInPlaces(const HttpRoute& r, uint32_t t)
{
    return r.inPlace[t].Count() ? r.inPlace[t] : defInPlaces[t];
}

void HttpProcessorImpl::setPlaces(const std::string& rs, const PlacementArray& places, HttpRequest& request, std::string& url)
{
    for(int i = 0; i < places.Count(); i++)
    {
        switch(places[i].type)
        {
            case PlacementType::PARAM:
                request.setQueryParameter(places[i].name, rs); break;
            case PlacementType::COOKIE:
                request.setCookie(places[i].name, rs); break;
            case PlacementType::HEADER:
                request.setHeaderField(places[i].name, rs); break;
            case PlacementType::URL:
                url += places[i].name + rs; break;
        }
    }
}

void HttpProcessorImpl::setFields(HttpRequest& request, HttpRoute& r)
{
    std::string URLField;
    char buf[20];
    buf[19] = 0;
    
    setPlaces(lltostr(request.getUSR(), buf + 19), getOutPlaces(r, PlacementKind::USR), request, URLField);
    setPlaces(request.getAbonent(), getOutPlaces(r, PlacementKind::ADDR), request, URLField);
    setPlaces(lltostr(request.getServiceId(), buf + 19), getOutPlaces(r, PlacementKind::SERVICE_ID), request, URLField);
    setPlaces(lltostr(request.getRouteId(), buf + 19), getOutPlaces(r, PlacementKind::ROUTE_ID), request, URLField);
   
    if(!URLField.length()) return;
    
    if(request.getSiteFileName().length())
        URLField = '/' + URLField;
        
    URLField += '/';
    
//    request.setSiteFileName(request.getSiteFileName() + URLField);
    request.setURLField(URLField);
}


bool HttpProcessorImpl::makeLongCall(HttpCommand& cmd, SessionPtr& se)
{
    LongCallContext& lcmCtx = se->getLongCallContext();
    lcmCtx.stateMachineContext = cmd.getContext();
    lcmCtx.initiator = HttpManager::Instance().getScagTaskManager();
	cmd.setSession(se);
    
    return LongCallManager::Instance().call(&lcmCtx);
}

int HttpProcessorImpl::processRequest(HttpRequest& request)
{
    HttpRoute r;

    RuleStatus rs;
    SessionPtr se;

    try{
        if(!request.hasSession())
        {
            smsc_log_debug(logger, "SERIALIZED REQUEST BEFORE PROCESSING: %s", request.serialize().c_str());
                
            if(!parsePath(request.getSitePath(), request))
            {
                registerEvent(scag::stat::events::http::REQUEST_FAILED, request);
                smsc_log_debug( logger, "http_request path parse error %s", request.getSitePath().c_str());
                request.setFailedBeforeSessionCreate(true);
                return scag::re::STATUS_FAILED;
            }
            try {
              if(request.getRouteId() > 0)
              {
                  r = router.findRouteByRouteId(request.getAbonent(), request.getRouteId(), request.getSitePath() + request.getSiteFileName());
                  request.setSite(r.defSite.host);
                  request.setSitePort(r.defSite.port);
              }
              else if(request.getServiceId() > 0)
              {
                  r = router.findRouteByServiceId(request.getAbonent(), request.getServiceId(), request.getSitePath() + request.getSiteFileName());
                  request.setSite(r.defSite.host);
                  request.setSitePort(r.defSite.port);
              }
              else
                  r = router.findRoute(request.getAbonent(), request.getSite(), request.getSitePath() + request.getSiteFileName(), request.getSitePort());        
            } catch (const RouteNotFoundException& e) {
              smsc_log_warn(logger, "Session not created. Route not found for abonent:%s, site:[%s]:[%d][%s][%s], route_id=%d, service_id=%d", request.getAbonent().c_str(), request.getSite().c_str(), request.getSitePort(), request.getSitePath().c_str(), request.getSiteFileName().c_str(), request.getRouteId(), request.getServiceId());
              registerEvent(scag::stat::events::http::REQUEST_FAILED, request);
              request.setFailedBeforeSessionCreate(true);
              request.trc.result = 404;
              return  scag::re::STATUS_FAILED;
            }
            
            if(r.id && !request.getAbonent().length())
            {
                smsc_log_info(logger, "Transit request served");
                registerEvent(scag::stat::events::http::REQUEST_OK, request);            
                return scag::re::STATUS_OK;
            }

            smsc_log_debug(logger, "Got http_request command host=%s:%d, path=%s, filename=%s, abonent=%s, USR=%d", request.getSite().c_str(), request.getSitePort(), request.getSitePath().c_str(), request.getSiteFileName().c_str(), request.getAbonent().c_str(), request.getUSR());
               
            smsc_log_debug( logger, "httproute found route_id=%d, service_id=%d", r.id, r.service_id);
            request.setServiceId(r.service_id);
            request.setRouteId(r.id);
            request.setProviderId(r.provider_id);

            findUSR(request, getInPlaces(r, PlacementKind::USR));

            const std::string& s = request.getAbonent();
            request.setAddress(r.addressPrefix + (s.c_str() + (s[0] == '+' ? 1 : 0)));

            CSessionKey sk = {request.getUSR(), request.getAddress().c_str()};
            if(!request.getUSR())
            {
                se = SessionManager::Instance().newSession(sk);
                request.setInitial();
                request.setUSR(sk.USR);
            }
            else
                if(!SessionManager::Instance().getSession(sk, se, request))
                    return scag::re::STATUS_PROCESS_LATER;
        }
        else
        {
            se = request.getSession();
            if(request.getRouteId() > 0)
                r = router.findRouteByRouteId(request.getAbonent(), request.getRouteId(), request.getSitePath() + request.getSiteFileName());
            else if(request.getServiceId() > 0)
                r = router.findRouteByServiceId(request.getAbonent(), request.getServiceId(), request.getSitePath() + request.getSiteFileName());
            else
                r = router.findRoute(request.getAbonent(), request.getSite(), request.getSitePath() + request.getSiteFileName(), request.getSitePort());        
        }
            
        if(se.Get())
        {
            RuleEngine::Instance().process(request, *se.Get(), rs);
            
            if(rs.status == scag::re::STATUS_LONG_CALL)
            {
                makeLongCall(request, se);
                return rs.status;
            }

            if(rs.status == scag::re::STATUS_OK)
            {
                registerEvent(scag::stat::events::http::REQUEST_OK, request);
                SessionManager::Instance().releaseSession(se);

                setFields(request, r);

                smsc_log_debug(logger, "SERIALIZED REQUEST AFTER PROCESSING: %s", request.serialize().c_str());
                return rs.status;
            }

            if (rs.status == scag::re::STATUS_FAILED) {
              request.trc.result = rs.result;
            }
        } else
            smsc_log_error( logger, "session not found for addr=%s, USR=%d", request.getAddress().c_str(), request.getUSR());
    }
    catch(RouteNotFoundException& e)
    {
        smsc_log_warn(logger, "route not found for abonent:%s, site:[%s]:[%d][%s][%s], route_id=%d, service_id=%d", request.getAbonent().c_str(), request.getSite().c_str(), request.getSitePort(), request.getSitePath().c_str(), request.getSiteFileName().c_str(), request.getRouteId(), request.getServiceId());
    }
    catch(Exception& e)
    {
        smsc_log_error(logger, "error processing request. %s", e.what());
    }
    catch(const std::runtime_error& e)
    {
      smsc_log_error( logger, "error processing request. runtime_error: %s", e.what());
    }

    if(se.Get())
        SessionManager::Instance().releaseSession(se);

    registerEvent(scag::stat::events::http::REQUEST_FAILED, request);

    return scag::re::STATUS_FAILED;
}

int HttpProcessorImpl::processResponse(HttpResponse& response)
{
    smsc_log_debug( logger, "Got http_response command abonent=%s, USR=%d, route_id=%d, service_id=%d", response.getAbonent().c_str(), response.getUSR(), response.getRouteId(), response.getServiceId());
    
    if(!response.getAbonent().length())
    {
        smsc_log_debug(logger, "Transit response served");
        return scag::re::STATUS_OK;
    }

    smsc_log_debug(logger, "SERIALIZED RESPONSE BEFORE: %s", response.serialize().c_str());
    
    SessionPtr se;
    try{
        if(!response.hasSession())
        {
            CSessionKey sk = {response.getUSR(), response.getAddress().c_str()};
            if(!SessionManager::Instance().getSession(sk, se, response))
                return scag::re::STATUS_PROCESS_LATER; // TODO: proper signal statemachine
        }
        else
            se = response.getSession();

        RuleStatus rs;

        if(se.Get())
        {
            RuleEngine::Instance().process(response, *se.Get(), rs);

            if(rs.status == scag::re::STATUS_OK)
            {
                registerEvent(scag::stat::events::http::RESPONSE_OK, response);
                response.setSession(SessionPtr(0));
                SessionManager::Instance().releaseSession(se);
                smsc_log_debug(logger, "SERIALIZED RESPONSE AFTER: %s", response.serialize().c_str());
                return rs.status;
            }

            if(rs.status == scag::re::STATUS_LONG_CALL)
            {
                makeLongCall(response, se);
                return rs.status;
            }
        } else
            smsc_log_error( logger, "http_response session not found abonent=%s, USR=%d", response.getAddress().c_str(), response.getUSR());
    }
    catch(Exception& e)
    {
        smsc_log_error( logger, "http_response error processing abonent=%s, USR=%d. %s", response.getAbonent().c_str(), response.getUSR(), e.what());
    }
    catch(const std::runtime_error& e)
    {
      smsc_log_error( logger, "http_response error processing abonent=%s, USR=%d. runtime_error: %s", response.getAbonent().c_str(), response.getUSR(), e.what());
    }
    catch(...)
    {
        smsc_log_error( logger, "http_response error processing abonent=%s, USR=%d.", response.getAbonent().c_str(), response.getUSR());
    }

    response.setSession(SessionPtr(0));

    if(se.Get())
        SessionManager::Instance().releaseSession(se);

    registerEvent(scag::stat::events::http::RESPONSE_FAILED, response);

    return scag::re::STATUS_FAILED;
}

int HttpProcessorImpl::statusResponse(HttpResponse& response, bool delivered)
{
    smsc_log_debug(logger, "Got http_status_response command abonent=%s, USR=%d, route_id=%d, service_id=%d, delivered=%d",
             response.getAbonent().c_str(), response.getUSR(), response.getRouteId(), response.getServiceId(), delivered);
             
    if(!response.getAbonent().length())
    {
        smsc_log_debug(logger, "Transit response served");
        return scag::re::STATUS_OK;
    }

    SessionPtr se;
    try{
        if(!response.hasSession())
        {
          CSessionKey sk = {response.getUSR(), response.getAddress().c_str()};
            if(!SessionManager::Instance().getSession(sk, se, response))
                return scag::re::STATUS_PROCESS_LATER;
        }
        else {
          se = response.getSession();
        }

        RuleStatus rs;
        if(se.Get())
        {
            response.setCommandId(HTTP_DELIVERY);
            response.setDelivered(delivered);
            RuleEngine::Instance().process(response, *se.Get(), rs);
            
            if(rs.status == scag::re::STATUS_OK && delivered)
            {
                registerEvent(scag::stat::events::http::DELIVERED, response, true);
                SessionManager::Instance().releaseSession(se);
                return rs.status;
            }

            if(rs.status == scag::re::STATUS_LONG_CALL)
            {
                makeLongCall(response, se);
                return rs.status;
            }
        }
        else
            smsc_log_error( logger, "http_status_response session not found abonent=%s, USR=%d", response.getAddress().c_str(), response.getUSR());
    }
    catch(Exception& e)
    {
        smsc_log_error( logger, "http_status_response error processing abonent=%s, USR=%d. %s", response.getAbonent().c_str(), response.getUSR(), e.what());
    }
    catch(const std::runtime_error& e)
    {
      smsc_log_error( logger, "http_status_response error processing abonent=%s, USR=%d. runtime_error: %s", response.getAbonent().c_str(), response.getUSR(), e.what());
    }
    catch(...)
    {
        smsc_log_error( logger, "Unknown exception. http_status_response error processing abonent=%s, USR=%d.", response.getAbonent().c_str(), response.getUSR());
    }

    if(se.Get())
        SessionManager::Instance().releaseSession(se);

    registerEvent(scag::stat::events::http::FAILED, response, true);
    
    return scag::re::STATUS_FAILED;
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

    router.getDefaultInPlacement(defInPlaces);
    router.getDefaultOutPlacement(defOutPlaces);
}

void HttpProcessorImpl::registerEvent(int event, HttpCommand& cmd, bool delivery)
{
    char buf[20];
    std::string s = cmd.getSite();
    if(cmd.getSitePort() != 80)
    {
        s += ":";
        sprintf(buf, "%d", cmd.getSitePort());
        s += buf;
    }
    buf[19] = 0;
    scag::stat::Statistics::Instance().registerEvent(scag::stat::HttpStatEvent(event, lltostr(cmd.getRouteId(), buf + 19), cmd.getServiceId(), cmd.getProviderId(), s, cmd.getSitePath() + cmd.getSiteFileName(), delivery ? cmd.getStatus() : 0 ));
}

}}}
