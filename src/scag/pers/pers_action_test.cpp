/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <logger/Logger.h>
#include <scag/re/RuleEngine.h>

#include "PersClient.h"
#include <scag/sessions/SessionManager.h>
#include <scag/config/ConfigManager.h>
#include <scag/transport/http/Managers.h>
#include <scag/stat/StatisticsManager.h>

using namespace std;
using namespace smsc::logger;
using namespace scag::re;
using namespace scag::transport::smpp;
using namespace scag::sessions;
using namespace scag::pers::client;
using scag::config::ConfigManager;
using namespace scag::transport;
using namespace scag::transport::smpp;
using namespace scag::transport::http;
using namespace scag::stat;

static Logger *logger;

extern "C" static void atExitHandler(void)
{
    Logger::Shutdown();
}
                                           
int main(int argc, char* argv[])
{
    int resultCode = 0;

    Logger::Init();
    logger = Logger::getInstance("test.persaction");

    atexit(atExitHandler);

    try{
        ConfigManager::Init();
        ConfigManager & cfg = ConfigManager::Instance();

        BillingManager::Init(cfg.getBillManConfig());

        SessionManager::Init(cfg.getSessionManConfig());

        PersClient::Init("127.0.0.1", 1200, 300);

        StatisticsManager::init(cfg.getStatManConfig());

        RuleEngine::Init("./rules");
        RuleEngine& re=RuleEngine::Instance();

        CSessionKey key;
        SMS sms;
        sms.originatingAddress = Address("+79139031234");
        sms.destinationAddress = Address("+79139031233");

        char buff[128];

        SmppCommand commandDeliver = SmppCommand::makeDeliverySm(sms,1);
        SmppCommand commandSubmit = SmppCommand::makeSubmitSm(sms,1);

        commandDeliver.setServiceId(1);
        commandDeliver.setServiceId(1);

        commandSubmit.setServiceId(1);

        SessionManager& sm = SessionManager::Instance();
        SessionPtr sessionPtr = sm.newSession(key);
        Session * session = sessionPtr.Get();

        if(session) smsc_log_warn(logger, "SESSION IS VALID");

        RuleEngine::Instance().process(commandDeliver, *session);
        RuleEngine::Instance().process(commandSubmit, *session);
    }
    catch (PersClientException& exc) 
    {
        smsc_log_error(logger, "PersClientException: %s Exiting.", exc.what());
        resultCode = -3;
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
