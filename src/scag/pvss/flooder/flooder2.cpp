#include <memory>
#include <signal.h>
#include "FlooderConfig.h"
#include "FlooderStat.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "scag/pvss/api/core/client/ClientConfig.h"
#include "scag/pvss/api/core/client/ClientCore.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "util/config/ConfigView.h"
#include "util/config/Manager.h"

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


void getConfig( smsc::logger::Logger* logger,
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
        clientConfig.setInactivityTime(300000); // in msec
        clientConfig.setConnectTimeout(10000);  // in msec
        clientConfig.setChannelQueueSizeLimit(1000); // the queue length
        clientConfig.setConnectionsCount(1);
        clientConfig.setMaxReaderChannelsCount(5);
        clientConfig.setMaxWriterChannelsCount(5);
        clientConfig.setReadersCount(1);
        clientConfig.setWritersCount(1);
        clientConfig.setProcessTimeout(1000);
    } catch ( ConfigException& e ) {
        smsc_log_error(logger, "cannot set default value: %s", e.what() );
        abort();
    }

    smsc::util::config::ConfigView cview(manager,"PvssClient");
    try {
        clientConfig.setHost( cview.getString("host") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.host> missed. Defaul value is %s", clientConfig.getHost().c_str());
    }
    try { 
        clientConfig.setPort( cview.getInt("port"));
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.port> missed. Defaul value is %d", int(clientConfig.getPort())&0xffff );
    }
    try { 
        clientConfig.setIOTimeout( cview.getInt("ioTimeout") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.ioTimeout> missed. Defaul value is %d", clientConfig.getIOTimeout());
    }
    try { 
        clientConfig.setInactivityTime( cview.getInt("pingTimeout")*1000 );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.pingTimeout> missed. Defaul value is %d / 1000", clientConfig.getInactivityTime() );
    }
    try { 
        clientConfig.setConnectTimeout( cview.getInt("reconnectTimeout")*1000 );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.reconnectTimeout> missed. Defaul value is %d / 1000", clientConfig.getConnectTimeout() );
    }
    try { 
        clientConfig.setChannelQueueSizeLimit( cview.getInt("maxWaitingRequestsCount") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.maxWaitingRequestsCount> missed. Defaul value is %d", clientConfig.getChannelQueueSizeLimit() );
    }
    try { 
        clientConfig.setConnectionsCount( cview.getInt("connections") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.connections> missed. Defaul value is %d", clientConfig.getConnectionsCount());
    }
    try { 
        unsigned connPerThread = cview.getInt("connPerThread");
        clientConfig.setMaxReaderChannelsCount( connPerThread );
        clientConfig.setMaxWriterChannelsCount( connPerThread );
        clientConfig.setReadersCount( (clientConfig.getConnectionsCount()-1) / connPerThread + 1 );
        clientConfig.setWritersCount( (clientConfig.getConnectionsCount()-1) / connPerThread + 1 );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.connPerThread> missed. Defaul value is %d/%d", clientConfig.getMaxReaderChannelsCount(), clientConfig.getMaxWriterChannelsCount());
    }
    
    try {
        flooderConfig.setAsyncMode(true);
        flooderConfig.setSpeed(1000);
        // flooderConfig.setGetSetCount(200);
        flooderConfig.setAddressesCount(1000000);
        flooderConfig.setFlooderThreadCount(clientConfig.getConnectionsCount());
        flooderConfig.setCommands("s0g0");
    } catch ( ConfigException& e ) {
        smsc_log_error(logger, "cannot set default value: %s", e.what() );
        abort();
    }

    smsc::util::config::ConfigView fview(manager,"Flooder");
    try {
        flooderConfig.setAsyncMode( cview.getBool("async") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <PvssClient.async> missed. Defaul value is %d", flooderConfig.getAsyncMode() );
    }
    try { 
        flooderConfig.setSpeed( fview.getInt("speed") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.speed> missed. Defaul value is %d", flooderConfig.getSpeed());
    }
    try { 
        flooderConfig.setAddressesCount( fview.getInt("addressesCount") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.addressesCount> missed. Defaul value is %d", flooderConfig.getAddressesCount());
    }
    try { 
        const unsigned propertiesCount = fview.getInt("properties");
        std::vector< std::string > properties;
        for ( unsigned i = 0; i < propertiesCount; ++i ) {
            char pbuf[50];
            snprintf(pbuf,sizeof(pbuf),"property.%d",i);
            properties.push_back( fview.getString(pbuf) );
        }
        flooderConfig.setPropertyPatterns(properties);
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.properties> missed or broken. Default number of properties is %d", flooderConfig.getPropertyPatterns().size());
    }

    if ( flooderConfig.getPropertyPatterns().size() <= 0 ) {
        smsc_log_error(logger, "cannot proceed: no property patterns specified");
        abort();
    }

    try {
        flooderConfig.setCommands( fview.getString("commands") );
    } catch (...) {
        smsc_log_warn(logger, "Parameter <Flooder.commands> missed. Default value is %s", flooderConfig.getCommands().c_str());
    }
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
    FlooderConfig flooderConfig;
    getConfig( logger, clientConfig, flooderConfig );

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
