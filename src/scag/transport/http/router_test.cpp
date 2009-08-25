#include "HttpRouter.h"
#include <logger/Logger.h>
#include <vector>

using namespace scag::transport::http;

using namespace smsc::logger;
using namespace smsc::util;

int main(int argc, char* argv[])
{
    Logger *logger;

    Logger::Init();

    logger = Logger::getInstance("router_test");

//    HttpRouterImpl rt;
    HttpTraceRouter::Init("http_routes.xml");

    try{
//        rt.init(".");
        HttpTraceRouter& tr = HttpTraceRouter::Instance();
        tr.ReloadRoutes();
        std::vector<std::string> trace;
        tr.getTraceRoute("79039004444", "mail.ru", "/melodies/rus/pop/3.mp3", 8080, trace);
        for(int i = 0; i< trace.size(); i++)
            smsc_log_debug(logger, trace[i].c_str());
        smsc_log_debug(logger, "sdfsdf");
        std::vector<std::string> trace1;
        tr.getTraceRouteById("79039004444", "/utf8/2.html", 1, 0, trace1);
        for(int i = 0; i< trace1.size(); i++)
            smsc_log_debug(logger, trace1[i].c_str());
/*        HttpRoute r = rt.findRoute("+79139034444", "yandex.ru", "/melodies", 8080);
        smsc_log_info(logger, "first test route found routeid=%s, service_id=%d", r.id.c_str(), r.service_id);
        r = rt.findRoute("+79609134444", "yandex.ru", "/melodies", 8080);*/
    }
    catch(Exception &e)
    {
        smsc_log_error(logger, "top level exception: %s", e.what());
    }
    catch(RouteNotFoundException &e)
    {
        smsc_log_error(logger, "second test route not found. ok.");
    }
    catch(...)
    {
        smsc_log_error(logger, "unknnown exception");
    }
}
