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

#include "TaskProcessor.h"
#include "tasks/PeriodicTask.h"
#include "tasks/DistributeTask.h"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;
using namespace smsc::system;

using namespace smsc::infosme;

static bool bInfoSmeIsStopped = false;
static bool bInfoIsConnected  = false;

static log4cpp::Category& logger = Logger::getCategory("smsc.infosme.InfoSme");

int main(void/*int argc, char* argv[]*/)
{
    using smsc::db::DataSourceLoader;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;

    PeriodicTaskFactory _periodicTaskFactory;
    TaskFactory::registerFactory(&_periodicTaskFactory, INFO_SME_PERIODIC_TASK_IDENTITY);
    DistributeTaskFactory _distributeTaskFactory;
    TaskFactory::registerFactory(&_distributeTaskFactory, INFO_SME_DISTRIBUTE_TASK_IDENTITY);

    try 
    {
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);
        
        ConfigView tpConfig(manager, "InfoSme");
        TaskProcessor processor(&tpConfig);

    }
    catch (SmppConnectException& exc)
    {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            logger.error("Failed to bind InfoSme. Exiting.\n");
        return -1;
    }
    catch (ConfigException& exc) 
    {
        logger.error("Configuration invalid. Details: %s Exiting.\n", exc.what());
        return -2;
    }
    catch (Exception& exc) 
    {
        logger.error("Top level Exception: %s Exiting.\n", exc.what());
        return -3;
    }
    catch (exception& exc) 
    {
        logger.error("Top level exception: %s Exiting.\n", exc.what());
        return -4;
    }
    catch (...) 
    {
        logger.error("Unknown exception: '...' caught. Exiting.\n");
        return -5;
    }
    
    return 0;
}
