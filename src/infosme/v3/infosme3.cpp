#include <cstdio>
#include <ctime>
#include <memory>
#include <signal.h>

#include "admin/service/ServiceSocketListener.h"
#include "logger/Logger.h"
#include "system/smscsignalhandlers.h"
#include "util/config/Manager.h"
#include "util/regexp/RegExp.hpp"
#include "util/xml/init.h"
#include "version.inc"

smsc::logger::Logger* mainlog = 0;

static sigset_t original_signal_mask;

extern "C" void appSignalHandler(int sig)
{
    smsc_log_info(mainlog,"signal handler invoked, sig=%d",sig);
    if ( sig == smsc::system::SHUTDOWN_SIGNAL || sig == SIGINT ) {
        // FIXME: notify to stop
        smsc_log_info(mainlog,"FIXME: stop notification");
    }
}


extern "C" void atExitHandler(void)
{
    smsc_log_info(mainlog,"at exit handler invoked");
    smsc::util::xml::TerminateXerces();
    smsc::logger::Logger::Shutdown();
}


/// check license file and return the number of SMS per second limit
unsigned checkLicenseFile()
{
    // FIXME
    return 100;
}


int main( int argc, char** argv )
{
    try {

        if ( argc > 1 ) {

            if (!strcmp(argv[1], "-h") || !strcmp(argv[1],"--help")) {
                printf("%s [options]\n",argv[0]);
                printf(" Options:\n");
                printf(" -h --help   \tThis help\n");
                printf(" -v --version\tShow program version\n");
                return 0;
            }

            if (!strcmp(argv[1], "-v") || !strcmp(argv[1],"--version")) {
                printf("%s\n", getStrVersion());
                return 0;
            }
        }

        // infrastructure
        smsc::util::regexp::RegExp::InitLocale();
        smsc::logger::Logger::Init();
        mainlog = smsc::logger::Logger::getInstance("infosme.main");
        smsc_log_info(mainlog,"Starting up %s",getStrVersion());

        std::auto_ptr< smsc::admin::service::ServiceSocketListener>
            adml( new smsc::admin::service::ServiceSocketListener() );
        // adminListener = adml.get();

        {
            atexit( atExitHandler );
            sigset_t set;
            sigemptyset(&set);
            sigprocmask(SIG_SETMASK, &set, &original_signal_mask);
            sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
            sigset(SIGINT, appSignalHandler);
            sigset(SIGPIPE, SIG_IGN);
        }

        try {

            // license
            checkLicenseFile();

            smsc::util::config::Manager::init("config.xml");
            {
                // FIXME: remove it
                struct timespec ts = {5,0};
                nanosleep(&ts,0);
            }

        } catch ( std::exception& e ) {

            smsc_log_error(mainlog,"error: %s", e.what());

        }

    } catch ( std::exception& e ) {
        fprintf(stderr,"exception caught %s\n",e.what());
        return -1;
    } catch ( ... ) {
        fprintf(stderr,"unknown exception caught\n");
        return -1;
    }
    return 0;
}
