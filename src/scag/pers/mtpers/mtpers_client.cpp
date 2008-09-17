/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <unistd.h>
#include <signal.h>
#include <logger/Logger.h>

#include "scag/pers/version.inc"

#include "core/threads/ThreadPool.hpp"

#include "scag/pers/mtpers/MTPersClient.h"

#include "scag/config/ConfigManager.h"

using namespace std;
using namespace smsc::logger;
using namespace scag::pers::util;
using namespace scag::pers;
using namespace scag::config;
using smsc::core::network::Socket;
using scag::mtpers::ClientTask;

using smsc::core::threads::ThreadPool;

#define ITER_CNT 2500

static ClientTask *mainTask = NULL;
bool run = true;

extern "C" void appSignalHandler(int sig) {

    Logger* logger = Logger::getInstance("client");
    smsc_log_debug(logger, "Signal %d handled !", sig);
    run = false;
    if (sig==SIGTERM || sig==SIGINT)
    {
        //pool.shutdown();
        if (mainTask) mainTask->stop();
        smsc_log_info(logger, "Stopping ...");
    }
}

extern "C" void atExitHandler(void)
{
    run = false;
    Logger::Shutdown();
    //pool.shutdown();
}

int main(int argc, char* argv[])
{
    int resultCode = 0;

    Logger::Init();
    Logger* logger = Logger::getInstance("client");

    atexit(atExitHandler);

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGTERM);
    sigdelset(&set, SIGINT);
    sigdelset(&set, SIGSEGV);
    sigdelset(&set, SIGBUS);
    sigdelset(&set, SIGHUP);

    sigprocmask(SIG_SETMASK, &set, NULL);
    sigset(SIGTERM, appSignalHandler);
    sigset(SIGINT, appSignalHandler);
    sigset(SIGHUP, appSignalHandler); 

    try {
        string host = "phoenix";
        int port = 47111;
        if (argc > 1) {
          port = atoi(argv[1]);
        }
        string address = "89138907759";
        if (argc > 2) {
          address = argv[3];
        }

        int threadCount = 0;

        ThreadPool pool;
        for (int i = 0; i < threadCount; ++i) {
          ClientTask *t = new ClientTask(host, port);
          pool.startTask(t);
        }
        mainTask = new ClientTask(host, port);
        mainTask->Execute();
        pool.shutdown();
        smsc_log_error(logger, "Test Stopped");
       
    }
    catch (PersClientException& exc) 
    {
        smsc_log_error(logger, "PersClientException: %s Exiting.", exc.what());
        resultCode = -2;
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
    catch (const SerialBufferOutOfBounds &e) 
    {
        smsc_log_error(logger, "SerialBufferOutOfBounds. Exiting.");
        resultCode = -5;
    }
    catch (...) 
    {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -6;
    }

    return resultCode;
}
