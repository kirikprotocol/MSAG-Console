#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>
#include <string>

#include "admin/service/ServiceSocketListener.h"
#include "core/buffers/File.hpp"
#include "core/threads/Thread.hpp"
#include "scag/util/Inst.h"
#include "scag2.h"
#include "scag/admin/gen2/SCAGSocketListener2.h"
#include "scag/config/impl/ConfigManager2.h"
#include "scag/util/singleton/LoggerSingleton.h"
#include "scag/util/singleton/XercesSingleton.h"
#include "scag/util/storage/StorageNumbering.h"
#include "util/signal.hpp"
#include "util/xml/init.h"
#include "version.inc"

static const int SHUTDOWN_SIGNAL = SIGTERM;

static int shutdownFlag = 0;
static sigset_t st;

static scag2::Scag *app = NULL;

scag2::Scag* getApp()
{
    return app;
}

extern "C" void sigHandler(int signo)
{
    fprintf(stderr,"MSAG Signal received %d\n", signo);
    shutdownFlag = signo;
}

void registerSignalHandlers()
{
  sigfillset(&st);
  sigdelset(&st, SIGBUS);
  sigdelset(&st, SIGFPE);
  sigdelset(&st, SIGILL);
  sigdelset(&st, SIGSEGV);
  sigdelset(&st, SIGINT);
  sigdelset(&st, SIGHUP);  
  sigprocmask(SIG_SETMASK, &st, NULL);

  sigdelset(&st, SIGUSR2);
  sigdelset(&st, SIGALRM);
  sigdelset(&st, SIGABRT);
  sigdelset(&st, SHUTDOWN_SIGNAL);

  sigset(SIGUSR2, sigHandler);
  sigset(SIGFPE,  sigHandler);
  sigset(SIGILL,  sigHandler);
  signal(SIGABRT, sigHandler);
  sigset(SIGALRM, sigHandler);
  sigset(SHUTDOWN_SIGNAL,  sigHandler);
}

extern "C" void atExitHandler(void)
{
    //smsc::util::xml::TerminateXerces();
    //smsc::logger::Logger::Shutdown();
}


namespace scag2 {
int main( int argc, char* argv[] )
{
    // Logger::Init();
    util::singleton::LoggerSingleton::Instance();
    // xerces will be invoked on demand
    // scag::util::singleton::XercesSingleton::Instance();

    // atexit(atExitHandler);
    registerSignalHandlers();

    try {
        smsc::logger::Logger *logger = Logger::getInstance("scag.main");

//    smsc_log_info(logger,  "SCAG configuration loading..." );
//        scag::config::ConfigManager::Init();
//        scag::config::ConfigManager& cfgs0 = scag::config::ConfigManager::Instance();

        {
            // initialization of config
            scag2::config::ConfigManagerImpl* cfg = 
                new scag2::config::ConfigManagerImpl();
            cfg->Init();
        }
        // config::ConfigManager::Init();
        config::ConfigManager& cfgs = config::ConfigManager::Instance();
//    smsc_log_info(logger,  "SCAG configuration is loaded" );

        smsc_log_info(logger,  "Starting up %s", getStrVersion());

        cfgs.checkLicenseFile();

        in_port_t servicePort = 0;
        try {
            servicePort = cfgs.getConfig()->getInt("admin.port");
        } catch (std::exception e) {
        }


        try {
            const unsigned nodes = cfgs.getConfig()->getInt("General.nodes");
            const unsigned storages = cfgs.getConfig()->getInt("General.storages");
            scag::util::storage::StorageNumbering::setInstance( nodes, storages );
        } catch ( std::exception& e) {
            smsc_log_error( logger, "FATAL ERROR: cannot set the number of nodes/storages: %s", e.what() );
            exit(-1);
        }

        unsigned mynode;
        try {
            mynode = cfgs.getConfig()->getInt("General.node");
        } catch ( std::exception& e ) {
            smsc_log_error( logger, "FATAL ERROR: cannot set the node number");
            exit(-1);
        }

        char * admin_host = 0;
        try
        {
            admin_host = cfgs.getConfig()->getString("admin.host");
        }
        catch (scag::config::ConfigException &c)
        {}

        int usesignals = 1;
        try {
            usesignals = cfgs.getConfig()->getInt("General.usesignals");
        } catch (std::exception& ) {
            usesignals = 1;
        }

        // For instance control
        char filename[20];
        sprintf(filename, "/tmp/scag.%d", servicePort);
        scag::util::Inst inst(filename);
        // Shutdown if there is instance allready.
        if(!inst.run()) {
            fprintf(stderr, "Instance is running already.\n");
            exit(-1);
        }

        app = new Scag;

        smsc_log_info(logger,  "Start initialization");
        try {
            app->init( mynode );
        } catch ( std::exception& exc ) {
            smsc_log_error(logger,"Exception in msag init: %s", exc.what());
            app->shutdown();
            delete app;
            app = 0;
            throw;
        } catch (...) {
            smsc_log_error(logger,"Unknown exception in msag init");
            app->shutdown();
            delete app;
            app = 0;
            throw;
        }

        admin::SCAGSocketListener *listener = NULL;

        if (servicePort != 0 && admin_host != 0) {
            listener = new admin::SCAGSocketListener;
            listener->init(admin_host, servicePort);
            listener->Start();
        }
        else
            smsc_log_warn(logger, "WARNING: admin port not specified, admin module disabled - smsc is not administrable");

        if ( ! usesignals ) {
            sigset_t oldmask;
            sigprocmask( SIG_SETMASK, &st, &oldmask );
            smsc::core::synchronization::EventMonitor mon;
            smsc::core::synchronization::MutexGuard mg(mon);
            while(!shutdownFlag) {
                mon.wait(1000);
                // smsc_log_debug(logger,"main rolling");
                if ( smsc::core::buffers::File::Exists("stop")) {
                    shutdownFlag = SHUTDOWN_SIGNAL;
                    // sigsuspend(&st); 
                    smsc_log_debug(logger, "MAIN: stop is found. flag=%d", shutdownFlag); 
                }
            }
            sigprocmask( SIG_SETMASK, &oldmask, NULL );
        } else {
            while(!shutdownFlag) {
                sigsuspend(&st); 
                smsc_log_debug(logger, "MAIN: sigsuspend exited. flag=%d", shutdownFlag); 
            }
        }

        if(listener)
        {
            if(shutdownFlag == SHUTDOWN_SIGNAL)
                listener->shutdown();
            else
                listener->abort();
            listener->WaitFor();
            delete listener;
        }

        if(shutdownFlag == SHUTDOWN_SIGNAL)
            app->shutdown();
        else if(shutdownFlag == SIGUSR2)
            app->abortScag();
        else
            app->dumpScag();

        delete app;

        smsc_log_info(logger, "All finished");
    }
    catch (AdminException &e)
    {
        fprintf(stderr,"top level exception: %s\n", e.what());
        exit(-2);
    }
    catch (smsc::util::Exception &e)
    {
        fprintf(stderr,"top level exception: %s\n", e.what());
        exit(-1);
    }
    catch(std::exception& e)
    {
        fprintf(stderr,"top level exception: %s\n", e.what());
        exit(-1);
    }
    catch(...)
    {
        fprintf(stderr,"FATAL EXCEPTION!\n");
        exit(-1);
    }
    //graceful shutdown
    //we don't need to restore stats on next startup in this case
    return 0;
}

} // namespace scag2


int main( int argc, char* argv[] )
{
    scag2::main( argc, argv );
    return 0;
}
