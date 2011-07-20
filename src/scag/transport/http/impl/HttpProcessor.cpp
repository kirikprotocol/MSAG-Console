#include <string>

#include "RouterTypes.h"
#include "HttpProcessor.h"
#include "HttpRouter.h"
#include "Managers.h"
#include "logger/Logger.h"
#include "scag/lcm/base/LongCallManager2.h"
#include "scag/re/base/RuleEngine2.h"
#include "scag/re/base/ActionContext2.h"
#include "scag/re/base/CommandBridge.h"
#include "scag/sessions/base/Operation.h"
#include "scag/sessions/base/SessionManager2.h"
#include "scag/stat/base/Statistics2.h"
#include "scag/util/lltostr.h"

namespace scag2 {
namespace transport {
namespace http {

using namespace sessions;
using smsc::util::Exception;
using scag2::re::actions::CommandProperty;


bool HttpProcessorImpl::parsePath(const std::string &path, HttpRequest& cx)
{
  const char    *pos = path.c_str(), *mid, *end;
  std::string   str, rs;
    unsigned int i;

  smsc_log_debug(logger, "parsePath: %s", pos);
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
    if(findPlace("ADDR", rs, defInPlaces[PlacementKind::ADDR], cx, inURLFields)) {
        cx.setAbonent(rs);
    }
    
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

void HttpProcessorImpl::clearPlaces(const PlacementArray& places, HttpRequest& request) {
  for(int i = 0; i < places.Count(); i++)
  {
      switch(places[i].type)
      {
          case PlacementType::PARAM:
              smsc_log_debug(logger, "del from param: %s", places[i].name.c_str());
              request.delQueryParameter(places[i].name);
              break;
          case PlacementType::COOKIE:
              smsc_log_debug(logger, "del from cookie: %s", places[i].name.c_str());
              request.delCookie(places[i].name);
              break;
          case PlacementType::HEADER:
              smsc_log_debug(logger, "del from header: %s", places[i].name.c_str());
              request.delHeaderField(places[i].name);
              break;
      }
  }
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
                return true;
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

/*
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
 */

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
    
    for (int i = 0; i < PLACEMENT_KIND_COUNT; ++i) {
      clearPlaces(defInPlaces[i], request);
    }
    // clearPlaces(r.inPlace[PlacementKind::USR], request);
    
    // setPlaces(lltostr(request.getUSR(), buf + 19), getOutPlaces(r, PlacementKind::USR), request, URLField);
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


bool HttpProcessorImpl::makeLongCall( HttpCommand& cmd, ActiveSession& se )
{
    LongCallContext& lcmCtx = se->getLongCallContext();
    lcmCtx.stateMachineContext = cmd.getContext();
    lcmCtx.initiator = scagmgr_;
    const bool b = LongCallManager::Instance().call(&lcmCtx);
    if (b) se.leaveLocked();
    return b;
}


int HttpProcessorImpl::processRequest(HttpRequest& request)
{
    HttpRoute r;

    re::RuleStatus rs;
    ActiveSession se;

    try{

        if ( !request.getSession() )
        {
            if (logger->isDebugEnabled()) {
                smsc_log_debug(logger, "SERIALIZED REQUEST BEFORE PROCESSING:\n%s", request.serialize().c_str());
                std::string params;
                if (request.getPostParams(params)) {
                  smsc_log_debug(logger, "POST params: '%s'", params.c_str());
                }
            }
                
            // NOTE (bukind): we have to fetch site path into a local variable
            // as it will be passed by reference into parsePath
            const std::string pathToParse = request.getSitePath();
            if(!parsePath(pathToParse, request))
            {
                registerEvent( stat::events::http::REQUEST_FAILED, request );
                smsc_log_debug( logger, "http_request path parse error %s", pathToParse.c_str());
                request.setFailedBeforeSessionCreate(true);
                return scag::re::STATUS_FAILED;
            }
            smsc_log_debug( logger, "http_request parse result abonent=%s", request.getAbonent().c_str());
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
                {
                    r = router.findRoute(request.getAbonent(), request.getSite(), request.getSitePath() + request.getSiteFileName(), request.getSitePort());
                }
                smsc_log_debug(logger, "Route %p found for abonent:%s, site:[%s]:[%d][%s][%s], route_id=%d, service_id=%d", r, request.getAbonent().c_str(), request.getSite().c_str(), request.getSitePort(), request.getSitePath().c_str(), request.getSiteFileName().c_str(), request.getRouteId(), request.getServiceId());
            } catch (const RouteNotFoundException& e) {
                smsc_log_warn(logger, "Session not created. Route not found for abonent:%s, site:[%s]:[%d][%s][%s], route_id=%d, service_id=%d", request.getAbonent().c_str(), request.getSite().c_str(), request.getSitePort(), request.getSitePath().c_str(), request.getSiteFileName().c_str(), request.getRouteId(), request.getServiceId());
                registerEvent( stat::events::http::REQUEST_FAILED, request);
                request.setFailedBeforeSessionCreate(true);
                request.trc.result = 404;
                return  scag::re::STATUS_FAILED;
            }

            request.setStatistics(r.statistics);
            
            if(r.id && !request.getAbonent().length())
            {
                smsc_log_debug(logger, "Transit request served");
                if (r.statistics) {
                  //TODO: register traffic info event for transit route
                  //TODO: how we can create SessionKey if request.getAbonent().length() == 0
                    smsc_log_debug(logger, "process transit request: register traffic info event, url=%s", request.getUrl().c_str() );
                    const SessionKey sessionKey( request.getAbonent() );
                    CommandProperty cp ( scag2::re::CommandBridge::getCommandProperty
                                         (request, sessionKey.address(),
                                          static_cast<uint8_t>(request.getOperationId())));
                    SessionPrimaryKey primaryKey(sessionKey);
                    timeval tv = { time(0), 0 };
                    primaryKey.setBornTime(tv);
                    scag2::re::CommandBridge::RegisterTrafficEvent(cp, primaryKey,
                                                                   request.getUrl().c_str(), 0);
                }
                registerEvent(stat::events::http::REQUEST_OK, request);            
                return scag::re::STATUS_OK;
            }

            smsc_log_debug(logger, "Got http_request command host=%s:%d, path=%s, filename=%s, abonent=%s", request.getSite().c_str(), request.getSitePort(), request.getSitePath().c_str(), request.getSiteFileName().c_str(), request.getAbonent().c_str());

            smsc_log_debug( logger, "httproute found route_id=%d, service_id=%d", r.id, r.service_id);
            request.setServiceId(r.service_id);
            request.setRouteId(r.id);
            request.setProviderId(r.provider_id);

            const std::string& s = request.getAbonent();
            request.setAddress(r.addressPrefix + (s.c_str() + (s[0] == '+' ? 1 : 0)));
        }
        else
        {
            if(request.getRouteId() > 0)
                r = router.findRouteByRouteId(request.getAbonent(), request.getRouteId(), request.getSitePath() + request.getSiteFileName());
            else if(request.getServiceId() > 0)
                r = router.findRouteByServiceId(request.getAbonent(), request.getServiceId(), request.getSitePath() + request.getSiteFileName());
            else
                r = router.findRoute(request.getAbonent(), request.getSite(), request.getSitePath() + request.getSiteFileName(), request.getSitePort());        
        }
            

        const SessionKey sessionKey( request.getAddress() );
        SCAGCommand* reqptr(&request);
        se = SessionManager::Instance().getSession( sessionKey, reqptr );

        if (se.get())
        {
            CommandProperty cp(scag2::re::CommandBridge::getCommandProperty(request, sessionKey.address(), static_cast<uint8_t>(request.getOperationId())));
            re::RuleEngine::Instance().process(request, *se.get(), rs, cp);
            HttpCommandRelease rel(request);
            
            if (rs.status == re::STATUS_LONG_CALL)
            {
                makeLongCall(request, se);
                rel.leaveLocked();
                return rs.status;
            }

            if (r.statistics) {
              smsc_log_debug(logger, "process request: register traffic info event, url=%s", request.getUrl().c_str());
              scag2::re::CommandBridge::RegisterTrafficEvent(cp, se->sessionPrimaryKey(), request.getUrl().c_str());
            }

            if (rs.status == re::STATUS_OK)
            {
                registerEvent(stat::events::http::REQUEST_OK, request);
                setFields(request, r);
                if (logger->isDebugEnabled()) {
                  smsc_log_debug(logger, "SERIALIZED REQUEST AFTER PROCESSING:\n%s", request.serialize().c_str());
                  std::string params;
                  if (request.getPostParams(params)) {
                    smsc_log_debug(logger, "POST params: '%s'", params.c_str());
                  }
                }
                return rs.status;
            }

            if (rs.status == re::STATUS_FAILED) {
                request.trc.result = rs.result;
            }
        } else {
            smsc_log_debug( logger, "session is locked for addr=%s", request.getAddress().c_str());
            return re::STATUS_PROCESS_LATER;
        }
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

    // if(se.Get())
    // SessionManager::Instance().releaseSession(se);
    registerEvent(stat::events::http::REQUEST_FAILED, request);
    return re::STATUS_FAILED;
}


int HttpProcessorImpl::processResponse(HttpResponse& response)
{
    smsc_log_debug( logger, "Got http_response command abonent=%s, route_id=%d, service_id=%d", response.getAbonent().c_str(), response.getRouteId(), response.getServiceId());
    
    if(!response.getAbonent().length())
    {
        smsc_log_debug(logger, "Transit response served");
        if (response.getStatistics()) {
          smsc_log_debug(logger, "process response: register traffic info event for transit route not implemented");
        }
        return re::STATUS_OK;
    }

    smsc_log_debug(logger, "SERIALIZED RESPONSE BEFORE: %s", response.serialize().c_str());
    
    ActiveSession se;
    try{

        /*
        if( !response.getSession() )
        {
            const SessionKey sk( response.getAddress() );
            SCAGCommand* rescmd(&response);
            if (!SessionManager::Instance().getSession(sk, se, response))
                return scag::re::STATUS_PROCESS_LATER; // TODO: proper signal statemachine
        }
        else
            se = response.getSession();
         */
        const SessionKey sk(response.getAddress());
        SCAGCommand* rescmd(&response);

        se = SessionManager::Instance().getSession( sk, rescmd, false );

        re::RuleStatus rs;
        if ( se.get() )
        {
            CommandProperty cp(scag2::re::CommandBridge::getCommandProperty(response, sk.address(), static_cast<uint8_t>(response.getOperationId())));
            HttpCommandRelease rel(response);
            re::RuleEngine::Instance().process(response, *se.get(), rs, cp);

            if (rs.status == re::STATUS_LONG_CALL)
            {
                makeLongCall(response, se);
                rel.leaveLocked();
                return rs.status;
            }

            if (response.getStatistics()) {
              smsc_log_debug(logger, "process response: register traffic info event, url=%s", response.getUrl().c_str() );
              scag2::re::CommandBridge::RegisterTrafficEvent(cp, se->sessionPrimaryKey(), response.getUrl().c_str() );
            }

            if (rs.status == re::STATUS_OK )
            {
                registerEvent(stat::events::http::RESPONSE_OK, response);
                // response.setSession(SessionPtr(0));
                // SessionManager::Instance().releaseSession(se);
                smsc_log_debug(logger, "SERIALIZED RESPONSE AFTER: %s", response.serialize().c_str());
                return rs.status;
            }


        } else {

            if ( ! rescmd ) {
                // session is locked by another command
                smsc_log_debug( logger, "http_response session is locked for abonent=%s", response.getAddress().c_str());
                return re::STATUS_PROCESS_LATER;
            } else {
                // session is not found
                smsc_log_error( logger, "http_response session not found abonent=%s", response.getAddress().c_str());
                // return re::STATUS_FAILED;
                rs.status = re::STATUS_FAILED;
            }
        }

        if (rs.status == re::STATUS_FAILED) {
            response.trc.result = rs.result;
        }

    }
    catch(Exception& e)
    {
        smsc_log_error( logger, "http_response error processing abonent=%s. %s", response.getAbonent().c_str(), e.what());
    }
    catch(const std::runtime_error& e)
    {
      smsc_log_error( logger, "http_response error processing abonent=%s. runtime_error: %s", response.getAbonent().c_str(), e.what());
    }
    catch(...)
    {
        smsc_log_error( logger, "http_response error processing abonent=%s.", response.getAbonent().c_str());
    }

    // response.setSession(SessionPtr(0));
    // if (se.Get())
    // SessionManager::Instance().releaseSession(se);

    registerEvent(stat::events::http::RESPONSE_FAILED, response);
    return re::STATUS_FAILED;
}

int HttpProcessorImpl::statusResponse(HttpResponse& response, bool delivered)
{
    smsc_log_debug(logger, "Got http_status_response command abonent=%s, route_id=%d, service_id=%d, delivered=%d",
             response.getAbonent().c_str(), response.getRouteId(), response.getServiceId(), delivered);
             
    if(!response.getAbonent().length())
    {
        smsc_log_debug(logger, "Transit status response served");
        if (response.getStatistics()) {
          smsc_log_debug(logger, "process status response: register traffic info event for transit route not implemented");
        }
        return re::STATUS_OK;
    }

    ActiveSession se;
    try{

        /*
        if (!response.getSession())
        {
            const SessionKey sk( response.getAddress() );
            if (!SessionManager::Instance().getSession(sk, se, response))
                return scag::re::STATUS_PROCESS_LATER;
        }
        else {
          se = response.getSession();
        }
         */
        const SessionKey sk( response.getAddress() );
        SCAGCommand* rescmd(&response);
        se = SessionManager::Instance().getSession(sk, rescmd, false);

        re::RuleStatus rs;
        if (se.get())
        {
            CommandProperty cp(scag2::re::CommandBridge::getCommandProperty(response, sk.address(), static_cast<uint8_t>(response.getOperationId())));
            HttpCommandRelease rel(response);
            response.setCommandId(HTTP_DELIVERY);
            response.setDelivered(delivered);
            re::RuleEngine::Instance().process(response, *se.get(), rs, cp);

            if(rs.status == re::STATUS_LONG_CALL)
            {
                makeLongCall(response, se);
                rel.leaveLocked();
                return rs.status;
            }

            if (response.getStatistics()) {
              smsc_log_debug(logger, "process status response: register traffic info event, url=%s", response.getUrl().c_str() );
              scag2::re::CommandBridge::RegisterTrafficEvent(cp, se->sessionPrimaryKey(), response.getUrl().c_str() );
            }
            
            if(rs.status == re::STATUS_OK && delivered)
            {
                registerEvent(stat::events::http::DELIVERED, response, true);
                // SessionManager::Instance().releaseSession(se);
                return rs.status;
            }

        }
        else {
          if ( ! rescmd ) {
              // session is locked by another command
              smsc_log_debug( logger, "http_response session is locked for abonent=%s", response.getAddress().c_str());
              return re::STATUS_PROCESS_LATER;
          } else {
              // session is not found
              smsc_log_error( logger, "http_response session not found abonent=%s", response.getAddress().c_str());
              // return re::STATUS_FAILED;
              rs.status = re::STATUS_FAILED;
          }
        }
    }
    catch(Exception& e)
    {
        smsc_log_error( logger, "http_status_response error processing abonent=%s. %s", response.getAbonent().c_str(), e.what());
    }
    catch(const std::runtime_error& e)
    {
      smsc_log_error( logger, "http_status_response error processing abonent=%s. runtime_error: %s", response.getAbonent().c_str(), e.what());
    }
    catch(...)
    {
        smsc_log_error( logger, "Unknown exception. http_status_response error processing abonent=%s.", response.getAbonent().c_str());
    }

    // if(se.Get())
    // SessionManager::Instance().releaseSession(se);

    registerEvent(stat::events::http::FAILED, response, true);
    return re::STATUS_FAILED;
}
        
void HttpProcessorImpl::init(const std::string& cfg)
{
    scagmgr_ = 0;
    logger = Logger::getInstance("http.proc");
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
    stat::Statistics::Instance().registerEvent(stat::HttpStatEvent(event, lltostr(cmd.getRouteId(), buf + 19), cmd.getServiceId(), cmd.getProviderId(), s, cmd.getSitePath() + cmd.getSiteFileName(), delivery ? cmd.getStatus() : 0 ));
}

}}}
