#include <memory>
#include <signal.h>
#include "FlooderConfig.h"
#include "FlooderStat.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "scag/pvss/api/core/client/ClientConfig.h"
#include "scag/pvss/api/core/client/impl/ClientCore.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "util/config/ConfigView.h"
#include "util/config/Manager.h"
#include "ConfigUtil.h"
#include "scag/util/Drndm.h"

using namespace scag2::pvss::core::client;
using namespace scag2::pvss::core;
using namespace scag2::pvss::flooder;
using namespace scag2::pvss;
using namespace scag2::exceptions;

smsc::core::synchronization::EventMonitor em;
std::auto_ptr<FlooderStat> flooderStat;
smsc::logger::Logger* logger = 0;

extern "C" void appSignalHandler(int sig)
{
    smsc_log_debug( logger, "Signal %d handled !", sig );
    if (sig==SIGTERM || sig==SIGINT)
    {
        if (flooderStat.get()) flooderStat->shutdown();
        smsc_log_info(logger, "Stopping ...");
        MutexGuard mg(em);
        em.notifyAll();
    }
    else if (sig == SIGHUP)
    {
        smsc_log_info(logger, "Reloading logger config");
        smsc::logger::Logger::Reload();
    }
}

extern "C" void atExitHandler(void)
{
    // temporary disabled as some long-living services requires logger after main exited.
    // smsc::logger::Logger::Shutdown();
}


void getConfig( smsc::logger::Logger* thelog,
                ClientConfig& clientConfig,
                FlooderConfig& flooderConfig )
{
    smsc::util::config::Manager::init("config.xml");
    smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();

    // setting default values
    try {
        clientConfig.setHost("phoenix");
        clientConfig.setPort(27880);
        clientConfig.setIOTimeout(300);
        clientConfig.setInactivityTime(30000); // in msec
        clientConfig.setConnectTimeout(10000);  // in msec
        clientConfig.setChannelQueueSizeLimit(1000); // the queue length
        clientConfig.setConnectionsCount(10);
        clientConfig.setMaxReaderChannelsCount(5);
        clientConfig.setMaxWriterChannelsCount(5);
        clientConfig.setReadersCount(2);
        clientConfig.setWritersCount(2);
        clientConfig.setProcessTimeout(1000);
    } catch ( ConfigException& e ) {
        smsc_log_error(thelog, "cannot set default value: %s", e.what() );
        fprintf(stderr,"cannot set default value: %s\n", e.what() );
        abort();
    }

    ::readClientConfig( thelog, clientConfig, manager );
    
    try {
        flooderConfig.setAsyncMode(true);
        flooderConfig.setSpeed(1000);
        // flooderConfig.setGetSetCount(200);
        flooderConfig.setAddressesCount(1000000);
        flooderConfig.setFlooderThreadCount(clientConfig.getConnectionsCount());
        flooderConfig.setCommands("s0g0");
        flooderConfig.setAddressFormat(".1.1.791%08u");
        flooderConfig.setOneCommandPerAbonent(false);
    } catch ( ConfigException& e ) {
        smsc_log_error(thelog, "cannot set default value: %s", e.what() );
        fprintf(stderr,"cannot set default value: %s\n", e.what() );
        abort();
    }

    ::readFlooderConfig( thelog, flooderConfig, manager );
}


int main( int argc, const char** argv )
{
    smsc::logger::Logger::Init();
    logger = smsc::logger::Logger::getInstance("main");
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

    ClientConfig clientConfig;
    FlooderConfig flooderConfig;
    getConfig( logger, clientConfig, flooderConfig );

    // --- reading speed from cmd line
    unsigned skip = unsigned(-1);
    if ( argc > 1 ) {
        unsigned newSpeed = unsigned(atoi(argv[1]));
        if ( newSpeed == 0 ) {
            fprintf(stderr, "wrong speed specified on Command line: %s\n", argv[1]);
            exit(-1);
        } else {
            printf("overriding flooder speed %u -> %u\n", flooderConfig.getSpeed(), newSpeed );
            flooderConfig.setSpeed( newSpeed );
        }
        
        if ( argc > 2 ) {
            skip = unsigned(atoi(argv[2]));
        }
    }

    // --- making a client
    std::auto_ptr< Client > client
        ( new ClientCore( new ClientConfig(clientConfig),
                          new scag2::pvss::pvap::PvapProtocol ));
    flooderStat.reset(new FlooderStat(flooderConfig,*client.get()));

    // randomizing the seed
    if ( skip == unsigned(-1) ) {
        // skip is not specified
        scag2::util::Drndm::getRnd().setSeed(uint64_t(time(0)));
    }

    try {
        flooderStat->init( skip );
    } catch ( IOException& e ) {
        smsc_log_error( logger, "cannot init flooder patterns: %s", e.what() );
        fprintf(stderr,"cannot init flooder patterns: %s\n", e.what());
        ::abort();
    }

    try {
        client->startup();
    } catch ( PvssException& e ) {
        smsc_log_error( logger, "client startup exception: %s", e.what() );
        fprintf(stderr,"client startup exception: %s\n", e.what());
        ::abort();
    }

    flooderStat->startup();

    /*
    for ( int i = 0; i < flooderConfig.getFlooderThreadCount(); ++i ) {
        smsc::core::threads::ThreadedTask* task;
        if ( flooderConfig.getAsyncMode() ) {
            task = new AsyncFlooderThread(*flooderStat.get(),*rg.get(),*client.get());
        } else {
            task = new SyncFlooderThread(*flooderStat.get(),*rg.get(),*client.get());
        }
        tp.startTask(task);
    }
     */

    flooderStat->waitUntilProcessed();
    flooderStat->shutdown();
    client->shutdown();
    return 0;
}
