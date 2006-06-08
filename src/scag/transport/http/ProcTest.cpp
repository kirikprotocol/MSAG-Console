/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <logger/Logger.h>
#include <scag/re/RuleEngine.h>

#include <scag/sessions/SessionManager.h>
#include <scag/config/ConfigManager.h>
#include <scag/transport/http/Managers.h>
#include <scag/stat/StatisticsManager.h>
#include "HttpProcessor.h"
#include "HttpRouter.h"

using namespace std;
using namespace smsc::logger;
using namespace scag::re;
using namespace scag::transport::smpp;
using namespace scag::sessions;
using scag::config::ConfigManager;
using namespace scag::transport;
using namespace scag::transport::smpp;
using namespace scag::transport::http;
using namespace scag::stat;

static Logger *logger;
Mutex mtx;
uint32_t req_cnt = 0, resp_cnt = 0, st_resp_cnt = 0;


extern "C" static void atExitHandler(void)
{
    Logger::Shutdown();
}
                                           
int main(int argc, char* argv[])
{
    HttpManager httpMan;

    int resultCode = 0;

    Logger::Init();
    logger = Logger::getInstance("httptest.proc");

    atexit(atExitHandler);

    try{
/*        HttpTraceRouter::Init("./conf/http_routes.xml");
        HttpTraceRouter& ttt = HttpTraceRouter::Instance();
        ttt.ReloadRoutes();
        smsc_log_debug(logger, ttt.getTraceRoute("79039126430", "192.168.1.213", "/", 80));
*/
        ConfigManager::Init();
        ConfigManager & cfg = ConfigManager::Instance();

        BillingManager::Init(cfg.getBillManConfig());

        SessionManager::Init(cfg.getSessionManConfig());

        StatisticsManager::init(cfg.getStatManConfig());

        RuleEngine::Init("./rules");
        RuleEngine& re=RuleEngine::Instance();

        smsc_log_info(logger, "Http Manager is starting");

        scag::transport::http::HttpProcessor::Init("./conf");
        scag::transport::http::HttpProcessor& hp = scag::transport::http::HttpProcessor::Instance();

        using scag::config::ConfigView;

        std::auto_ptr<ConfigView> cv(new ConfigView(*cfg.getConfig(), "HttpTransport"));

        scag::transport::http::HttpManagerConfig http_cfg(10, 10, 10, 10, 10, 10, 100, "", 1234);

        auto_ptr <char> host(cv->getString("host", NULL, false));
        http_cfg.host = host.get();

        http_cfg.readerSockets = cv->getInt("readerSockets", NULL);
        http_cfg.writerSockets = cv->getInt("writerSockets", NULL);
        http_cfg.readerPoolSize = cv->getInt("readerPoolSize", NULL);
        http_cfg.writerPoolSize = cv->getInt("writerPoolSize", NULL);
        http_cfg.scagPoolSize = cv->getInt("scagPoolSize", NULL);
        http_cfg.scagQueueLimit = cv->getInt("scagQueueLimit", NULL);
        http_cfg.connectionTimeout = cv->getInt("connectionTimeout", NULL);
        http_cfg.port = cv->getInt("port", NULL);
        smsc_log_debug(logger, "http_host=%s, http_port=%d", http_cfg.host.c_str(), http_cfg.port);

        httpMan.init(hp, http_cfg);

        smsc_log_info(logger, "Http Manager started");

        int k = 0;
#if 1
        do{
             sleep(1);
       }
        while (!(k == 'q' || k == 'Q'));
#else    
        sleep(60);
#endif    

        httpMan.shutdown();
        Logger::Shutdown();

    }
    catch (SCAGException& exc) 
    {
        smsc_log_error(logger, "SCAG Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (Exception& exc) 
    {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) 
    {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    }
    catch (...) 
    {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -5;
    }

    return resultCode;
}
