#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Array.hpp>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/recoder/recode_dll.h>
#include <util/smstext.h>

#include <signal.h>

#include <db/DataSourceLoader.h>

#include <system/smscsignalhandlers.h>
#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <util/xml/init.h>

#include <admin/service/Component.h>
#include <admin/service/ComponentManager.h>
#include <admin/service/ServiceSocketListener.h>

#include "TaskProcessor.h"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;
using namespace smsc::system;

using namespace smsc::admin;
using namespace smsc::admin::service;

using namespace smsc::infosme;

static bool bInfoSmeIsStopped = false;
static bool bInfoIsConnected  = false;

static log4cpp::Category& logger = Logger::getCategory("smsc.infosme.InfoSme");

static smsc::admin::service::ServiceSocketListener adminListener; 
static bool bAdminListenerInited = false;

class InfoSmeConfig : public SmeConfig
{
private:
    
    char *strHost, *strSid, *strPassword, *strSysType, *strOrigAddr;

public:
    
    InfoSmeConfig(ConfigView* config)
        throw(ConfigException)
            : SmeConfig(), strHost(0), strSid(0), strPassword(0), 
                strSysType(0), strOrigAddr(0)
    {
        // Mandatory fields
        strHost = config->getString("host", "SMSC host wasn't defined !");
        host = strHost;
        strSid = config->getString("sid", "WSme id wasn't defined !");
        sid = strSid;
        
        port = config->getInt("port", "SMSC port wasn't defined !");
        timeOut = config->getInt("timeout", "Connect timeout wasn't defined !");
        
        // Optional fields
        try
        {
            strPassword = config->getString("password",
                                            "WSme password wasn't defined !");
            password = strPassword;
        }
        catch (ConfigException& exc) { password = ""; strPassword = 0; }
        try
        {
            strSysType = config->getString("systemType", 
                                           "WSme system type wasn't defined !");
            systemType = strSysType;
        }
        catch (ConfigException& exc) { systemType = ""; strSysType = 0; }
        try
        {
            strOrigAddr = config->getString("origAddress", 
                                            "WSme originating address wasn't defined !");
            origAddr = strOrigAddr;
        }
        catch (ConfigException& exc) { origAddr = ""; strOrigAddr = 0; }
    };

    virtual ~InfoSmeConfig()
    {
        if (strHost) delete strHost;
        if (strSid) delete strSid;
        if (strPassword) delete strPassword;
        if (strSysType) delete strSysType;
        if (strOrigAddr) delete strOrigAddr;
    };
};

static void appSignalHandler(int sig)
{
    __trace2__("Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        __trace__("Stopping ...");
        if (bAdminListenerInited) adminListener.shutdown();
        bInfoSmeIsStopped = true;
    }
}
void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
}

int main(void)
{
    int resultCode = 0;

    Logger::Init("log4cpp.infosme");
    atexit(atExitHandler);

    try 
    {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
        sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
        
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);
        
        ConfigView tpConfig(manager, "InfoSme");
        TaskProcessor processor(&tpConfig);
        
        ConfigView smscConfig(manager, "WSme.SMSC");
        InfoSmeConfig cfg(&smscConfig);
        
        processor.Start();
        Event aaa;
        aaa.Wait(100000);
    }
    catch (SmppConnectException& exc)
    {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            logger.error("Failed to bind InfoSme. Exiting.\n");
        resultCode = -1;
    }
    catch (ConfigException& exc) 
    {
        logger.error("Configuration invalid. Details: %s Exiting.\n", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc) 
    {
        logger.error("Top level Exception: %s Exiting.\n", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) 
    {
        logger.error("Top level exception: %s Exiting.\n", exc.what());
        resultCode = -4;
    }
    catch (...) 
    {
        logger.error("Unknown exception: '...' caught. Exiting.\n");
        resultCode = -5;
    }
    
    if (bAdminListenerInited)
    {
        adminListener.shutdown();
        adminListener.WaitFor();
    }
    return resultCode;
}
