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
#include <scag/bill/BillingManager.h>
#include <scag/pers/PersClient.h>
#include "HttpProcessor.h"
#include "HttpRouter.h"
#include "scag/exc/SCAGExceptions.h"

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
using namespace scag::pers::client;
using namespace scag::bill;
using namespace scag::exceptions;

static Logger *logger;
Mutex mtx;
uint32_t req_cnt = 0, resp_cnt = 0, st_resp_cnt = 0;


extern "C" void atExitHandler(void)
{
    Logger::Shutdown();
}
                                           
int main(int argc, char* argv[])
{
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

        
        PersClient::Init(cfg.getPersClientConfig());
        
        BillingManager::Init(cfg.getBillManConfig());

        SessionManager::Init(cfg.getSessionManConfig());

        StatisticsManager::init(cfg.getStatManConfig());

        RuleEngine::Init("./rules");
        RuleEngine& re=RuleEngine::Instance();

        smsc_log_info(logger, "Http Manager is starting");

        scag::transport::http::HttpProcessor::Init("./conf");
        scag::transport::http::HttpProcessor& hp = scag::transport::http::HttpProcessor::Instance();

        HttpManager::Init(hp, cfg.getHttpManConfig());
        HttpManager& httpMan = HttpManager::Instance();

        sleep(5);
        cfg.reloadAllConfigs();
        
        int k = 0;
#if 1
        do{
             sleep(10);
       }
        while (!(k == 'q' || k == 'Q'));
#else    
        sleep(10);
#endif    
        smsc_log_debug(logger, "Http man shutdown");
        httpMan.shutdown();
        smsc_log_debug(logger, "after Http man shutdown");        
//        Logger::Shutdown();
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

