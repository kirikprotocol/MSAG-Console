#include "HttpRouter.h"
#include <logger/Logger.h>

using namespace scag::transport::http;

using namespace smsc::logger;

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
        smsc_log_debug(logger, tr.getTraceRoute("+79139034444", "yandex.ru", "/melodies", 8080).c_str());
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
