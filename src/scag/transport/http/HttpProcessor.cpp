#include <string>

#include <scag/re/RuleEngine.h>
#include <scag/sessions/SessionManager.h>
#include <scag/stat/Statistics.h>

#include "HttpCommand.h"
#include "HttpProcessor.h"

namespace scag { namespace transport { namespace http {

using namespace scag::sessions;
using namespace scag::re;

bool HttpProcessorImpl::processRequest(HttpRequest& request)
{
    HttpRoute r;
    try{
        r = router.findRoute(request.getAbonent(), request.getSite() + '/' + request.getSitePath());
        request.setServiceId(r.service_id);
        request.setRouteId(r.id);
    }
    catch(RouteNotFoundException& e)
    {
        return false;
    }

    CSessionKey sk = {request.getUSR(), request.getAbonent().c_str()};
    SessionPtr se = SessionManager::Instance().getSession(sk);

    if(!se.Get())
        return false;
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
    CSessionKey sk = {response.getUSR(), response.getAbonent().c_str()};
    SessionPtr se = SessionManager::Instance().getSession(sk);

    if(!se.Get())
        return false;

//    Statistics::Instance().registerEvent(HttpStatEvent(0, response.getRouteId(), "", -1));

    RuleStatus rs = RuleEngine::Instance().process(response, *se.Get());

//    Statistics::Instance().registerEvent(HttpStatEvent(0, response.getRouteId(), "", -1, rs.result));

    if(rs.result < 0)
        return false;

    return true;
}

void HttpProcessorImpl::statusResponse(HttpResponse& response, bool delivered)
{
    CSessionKey sk = {response.getUSR(), response.getAbonent().c_str()};
    SessionPtr se = SessionManager::Instance().getSession(sk);

//    Statistics::Instance().registerEvent(HttpStatEvent(0, response.getRouteId(), "", -1));

    response.setCommandId(HTTP_DELIVERY);

    RuleStatus rs = RuleEngine::Instance().process(response, *se.Get());
}
        
void HttpProcessorImpl::init(std::string& cfg)
{
    router.init(cfg);
}

}}}
