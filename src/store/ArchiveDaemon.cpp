
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

ArchiveProcessor* archiveProcessor = 0;

static void setNeedStop(bool stop=true) {
    MutexGuard gauard(needStopLock);
    bServiceIsStopped = stop;
    if (stop) {
        if (archiveProcessor) archiveProcessor->Stop();
        bServiceWaitEvent.Signal();
    }
}
static bool isNeedStop() {
    MutexGuard gauard(needStopLock);
    return bServiceIsStopped;
}

extern "C" void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT)
    {
        smsc_log_info(logger, "Stopping ...");
        setNeedStop(true);
    }
}
extern "C" void atExitHandler(void)
{
    //sigsend(P_PID, getppid(), SIGCHLD);
    smsc::util::xml::TerminateXerces();
}

extern "C" void setShutdownHandler(void)
{
    sigset_t set;
    sigemptyset(&set); 
    sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
    if(pthread_sigsetmask(SIG_UNBLOCK, &set, NULL) != 0) {
        if (logger) smsc_log_error(logger, "Failed to set signal mask (shutdown handler)");
    }
    sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
}
extern "C" void clearSignalMask(void)
{
    sigset_t set;
    sigemptyset(&set);
    for(int i=1;i<=37;i++)
        if(i!=SIGQUIT && i!=SIGBUS && i!=SIGSEGV) sigaddset(&set,i);
    if(pthread_sigsetmask(SIG_SETMASK, &set, NULL) != 0) {
        if (logger) smsc_log_error(logger, "Failed to clear signal mask");
    }
}

struct ShutdownThread : public Thread
{
    Event shutdownEvent;

    ShutdownThread() : Thread() {};
    virtual ~ShutdownThread() {};

    virtual int Execute() {
        clearSignalMask();
        setShutdownHandler();
        shutdownEvent.Wait();
        return 0;
    };
    void Stop() {
        shutdownEvent.Signal();    
    };
} shutdownThread;

int main(void)
{
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    Logger::Init();
    logger = Logger::getInstance("smsc.store.ArchiveDaemon");

    atexit(atExitHandler);
    clearSignalMask();
    shutdownThread.Start();
    
    int resultCode = 0;
    try 
    {
        smsc_log_info(logger, getStrVersion());
        Manager::init("daemon.xml");
        
        ConfigView apConfig(Manager::getInstance(), "ArchiveDaemon");
        int daemonInterval = apConfig.getInt("interval");
        ArchiveProcessor processor(&apConfig);
        archiveProcessor = &processor;

        bool first = true;
        while (!isNeedStop())
        {
            if (!first) bServiceWaitEvent.Wait(daemonInterval*1000);
            else first = false;
            processor.process();
        }

        archiveProcessor = 0;

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

    shutdownThread.Stop();
    return resultCode;
}
