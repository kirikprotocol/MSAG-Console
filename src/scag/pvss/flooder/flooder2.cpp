#include <signal.h>
#include <memory>
#include "logger/Logger.h"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "FlooderConfig.h"
#include "FlooderStat.h"
#include "scag/pvss/api/core/client/ClientConfig.h"
#include "scag/pvss/api/core/client/ClientCore.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"

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
    smsc::logger::Logger::Shutdown();
}

int main()
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
    try {
        clientConfig.setHost("127.0.0.1");
        clientConfig.setPort((short)1234);
        clientConfig.setConnectionsCount(1);
        clientConfig.setReadersCount(1);
        clientConfig.setWritersCount(1);
        clientConfig.setMaxReaderChannelsCount(2);
        clientConfig.setMaxWriterChannelsCount(2);
        clientConfig.setChannelQueueSizeLimit(100);
        clientConfig.setConnectTimeout(1000);
        clientConfig.setInactivityTime(15000);
        clientConfig.setProcessTimeout(1000);
        clientConfig.setIOTimeout(500);
    } catch ( ConfigException& e ) {
        smsc_log_error( logger, "exception on clientconfig: %s", e.what() );
        ::abort();
    }

    FlooderConfig flooderConfig;
    try {
        flooderConfig.setAsyncMode(false);
        flooderConfig.setSpeed(10);
        flooderConfig.setGetSetCount(200);
        flooderConfig.setAddressesCount(1000);
        flooderConfig.setFlooderThreadCount(1);
    } catch ( ConfigException& e ) {
        smsc_log_error( logger, "exception on clientconfig: %s", e.what() );
        ::abort();
    }

    // --- making a client
    std::auto_ptr< Protocol > protocol( new scag2::pvss::pvap::PvapProtocol );
    std::auto_ptr< Client > client( new ClientCore( clientConfig, *protocol.get() ) );

    try {
        client->startup();
    } catch ( PvssException& e ) {
        smsc_log_error( logger, "client startup exception: %s", e.what() );
        ::abort();
    }

    flooderStat.reset(new FlooderStat(flooderConfig,*client.get()));

    try {
        flooderStat->startup();
    } catch ( IOException& e ) {
        smsc_log_error( logger, "cannot parse flooder patterns: %s", e.what() );
        ::abort();
    }

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
