#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>
#include <string>

#include "gen2/Inst.h"
#include "admin/SCAGSocketListener2.h"
#include "admin/service/ServiceSocketListener.h"
#include "scag/config/ConfigManager.h"
#include "scag/config/impl/ConfigManager2.h"
#include "core/threads/Thread.hpp"
#include "gen2/scag2.h"
#include "util/config/route/RouteConfig.h"
#include "util/findConfigFile.h"
#include "util/mirrorfile/mirrorfile.h"
#include "util/signal.hpp"
#include "util/xml/init.h"
#include "version.inc"
#include "scag/util/storage/StorageNumbering.h"

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
    fprintf(stderr,"Signal received %d\n", signo);
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

  sigdelset(&st,17);
  sigdelset(&st, SIGALRM);
  sigdelset(&st, SIGABRT);
  sigdelset(&st, SHUTDOWN_SIGNAL);

  sigset(17,      sigHandler);
  sigset(SIGFPE,  sigHandler);
  sigset(SIGILL,  sigHandler);
  signal(SIGABRT, sigHandler);
  sigset(SIGALRM, sigHandler);
  sigset(SHUTDOWN_SIGNAL,  sigHandler);
}

extern "C" void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
    //smsc::logger::Logger::Shutdown();
}


namespace scag2 {
int main( int argc, char* argv[] )
{
    Logger::Init();

    atexit(atExitHandler);
    registerSignalHandlers();

    try {
        smsc::logger::Logger *logger = Logger::getInstance("scagmain");

//    smsc_log_info(logger,  "SCAG configuration loading..." );
        scag::config::ConfigManager::Init();
        scag::config::ConfigManager& cfgs0 = scag::config::ConfigManager::Instance();

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


        // FIXME: make it configurable
        scag::util::storage::StorageNumbering::setInstance( 1 );
        const unsigned mynode = 0;

        char * admin_host = 0;
        try
        {
            admin_host = cfgs.getConfig()->getString("admin.host");
        }
        catch (scag::config::ConfigException &c)
        {}

        // For instance control
        char filename[20];
        sprintf(filename, "/tmp/scag.%d", servicePort);
        Inst inst(filename);
        // Shutdown if there is instance allready.
        if(!inst.run()) {
            fprintf(stderr, "Instance is running already.\n");
            exit(-1);
        }

        app = new Scag;

        smsc_log_info(logger,  "Start initialization");
        app->init( mynode );

        admin::SCAGSocketListener *listener = NULL;

        if (servicePort != 0 && admin_host != 0) {
            listener = new admin::SCAGSocketListener;
            listener->init(admin_host, servicePort);
            listener->Start();
        }
        else
            smsc_log_warn(logger, "WARNING: admin port not specified, admin module disabled - smsc is not administrable");

        while(!shutdownFlag) { 
            sigsuspend(&st); 
            smsc_log_debug(logger, "MAIN: sigsuspend exited. flag=%d", shutdownFlag); 
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
        else if(shutdownFlag == 17)
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


int main(int argc,char* argv[])
{
    scag2::main( argc, argv );
}
