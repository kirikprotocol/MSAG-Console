
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <system/smscsignalhandlers.h>

#include <sms/sms.h>
#include <util/xml/init.h>

#include "version.inc"
#include "ArchiveProcessor.h"

using namespace smsc::util;

using namespace smsc::system;
using namespace smsc::store;

static smsc::logger::Logger* logger = 0;

static Mutex needStopLock;
static Event bServiceWaitEvent;
static bool  bServiceIsStopped = false;

static void setNeedStop(bool stop=true) {
    MutexGuard gauard(needStopLock);
    bServiceIsStopped = stop;
    if (stop) bServiceWaitEvent.Signal();
}
static bool isNeedStop() {
    MutexGuard gauard(needStopLock);
    return bServiceIsStopped;
}

extern "C" void signalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT) {
        smsc_log_info(logger, "Stopping ...");
        setNeedStop(true);
    }
    if (sig==SIGPIPE) {
        smsc_log_warn(logger, "Sig Pipe received !!!");
    }
}
extern "C" void atExitHandler(void)
{
    //sigsend(P_PID, getppid(), SIGCHLD);
    smsc::util::xml::TerminateXerces();
}

int main(void)
{
    Logger::Init("logger.properties");
    logger = Logger::getInstance("smsc.store.ArchiveDaemon");

    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    atexit(atExitHandler);
    
    sigset_t set; sigemptyset(&set);
    sigaddset(&set, SIGINT); sigaddset(&set, SIGPIPE); sigaddset(&set, SIGTERM);
    sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
    pthread_sigmask(SIG_SETMASK, &set, NULL);
    sigset(SIGINT, signalHandler); sigset(SIGTERM, signalHandler); sigset(SIGPIPE, signalHandler);
    sigset(smsc::system::SHUTDOWN_SIGNAL, signalHandler);
    
    int resultCode = 0;
    try 
    {
        smsc_log_info(logger, getStrVersion());
        Manager::init("daemon.xml");
        
        ConfigView apConfig(Manager::getInstance(), "ArchiveDaemon");
        int daemonInterval = apConfig.getInt("interval");
        ArchiveProcessor processor(&apConfig);

        while (!isNeedStop())
        {
            processor.process();
            bServiceWaitEvent.Wait(daemonInterval*1000);
        }

    } catch (ConfigException& exc) {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -1;
    } catch (Exception& exc) {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -2;
    } catch (std::exception& exc) {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -3;
    } catch (...) {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -4;
    }

    return resultCode;
}
