#include <memory>
#include <typeinfo>

#include "core/synchronization/Event.hpp"
#include "logger/Logger.h"
#include "scag/bill/impl/BillingManager.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/config/base/ConfigView.h"
#include "scag/lcm/impl/LongCallManagerImpl.h"

#include "scag/pvss/api/core/client/impl/ClientCore.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "scag/pvss/flooder/ConfigUtil.h"

#include "scag/re/base/XMLHandlers2.h" // for StrX
#include "scag/re/impl/RuleEngine2.h"
#include "scag/sessions/impl/SessionManager2.h"
#include "scag/stat/impl/StatisticsManager.h"
#include "scag/transport/http/impl/HttpRouter.h"
#include "scag/transport/http/impl/HttpProcessor.h"
#include "scag/transport/http/impl/Managers.h"
#include "scag/transport/smpp/impl/SmppManager2.h"
#include "scag/transport/smpp/router/load_routes.h"
#include "scag/util/encodings/Encodings.h"
#include "scag2.h"
#include "util/Exception.hpp"
#include "util/debug.h"
#include "util/findConfigFile.h"
#include "util/findConfigFile.h"
#include "util/regexp/RegExp.hpp"
#include "util/xml/utilFunctions.h"

#include "scag/counter/impl/HashCountManager.h"
#include "scag/counter/impl/TemplateManagerImpl.h"
#include "scag/counter/ActionTable.h"

namespace {

using namespace scag2;
using namespace scag2::transport;
using namespace scag2::transport::smpp;
using namespace scag2::transport::http;

class FromSessionQueue : public SCAGCommandQueue
{
public:
    FromSessionQueue() {}

    virtual unsigned pushCommand( SCAGCommand* cmd, int action = PUSH )
    {
        if ( ! cmd ) return unsigned(-1);
        switch ( cmd->getType() ) {
        case SMPP: {
            return SmppManager::Instance().pushSessionCommand
                ( static_cast<SmppCommand*>(cmd), action );
        }

        case HTTP: {
          return HttpManager::Instance().pushSessionCommand
              ( static_cast<HttpCommand*>(cmd), action );
        }

        default : {
            throw std::runtime_error("transports other than SMPP and HTTP is not implemented yet");
        }
        } // switch
        return unsigned(-1);
    }

    /// --- the following methods are not used
    virtual void stop() {
    }

    virtual bool isStopping() const {
        return false;
    }

};

FromSessionQueue fsq;

smsc::logger::Logger* log;

} // namespace


namespace scag2 {

using namespace scag::exceptions;
using std::auto_ptr;
using std::string;
using std::exception;
using namespace smsc::sms;
using namespace smsc::core::synchronization;
using smsc::util::Exception;
using config::ConfigManager;
using smsc::util::findConfigFile;
using namespace xercesc;



Scag::Scag() :
stopFlag(false),
testRouter_(0),
pvssInited_(false),
lcmInited_(false),
billInited_(false),
httpInited_(false),
smppInited_(false),
sessInited_(false),
statInited_(false)
{
    startTime=0;
};


Scag::~Scag()
{
}

void Scag::init( unsigned mynode )
{
    log = smsc::logger::Logger::getInstance("scag.init");

    smsc_log_info(log, "SCAG start initialisation...");

    ConfigManager& cfg = ConfigManager::Instance();

    //************** Personalization client initialization **************

    try {
        smsc_log_info(log,"creating counter manager...");
        counter::impl::HashCountManager* mgr =
            new counter::impl::HashCountManager
            ( new counter::impl::TemplateManagerImpl(), 10 );
        mgr->start();
    } catch (...) {
        smsc_log_error(log,"exc in counter mgr");
        abort();
    }

    {
        smsc_log_info(log,"Adding a few counter templates");
        counter::Manager& mgr = counter::Manager::getInstance();
        counter::TemplateManager* tmgr = mgr.getTemplateManager();
        if (!tmgr) {
            throw SCAGException("no template manager is found in counter manager");
        }
        {
            const char* cname = "sys.traffic.global.smpp";
            counter::ActionTable* o = new counter::ActionTable();
            counter::ActionList* al = new counter::ActionList();
            al->push_back(counter::ActionLimit(95,counter::GT,counter::CRITICAL));
            al->push_back(counter::ActionLimit(90,counter::GT,counter::MAJOR));
            al->push_back(counter::ActionLimit(80,counter::GT,counter::MINOR));
            al->push_back(counter::ActionLimit(70,counter::GT,counter::WARNING));
            o->setNewActions(al);
            tmgr->replaceObserver(cname,o);
            tmgr->replaceTemplate( cname,
                                   counter::CounterTemplate::create
                                   ("timesnapshot", o, 10, 20));
        }
        {
            const char* names[] = { "sys.traffic.smpp.sme", "sys.traffic.smpp.smsc", 0 };
            for ( const char** cname = names; *cname != 0; ++cname ) {
                counter::ActionTable* o = new counter::ActionTable();
                counter::ActionList* al = new counter::ActionList();
                al->push_back(counter::ActionLimit(95,counter::GT,counter::CRITICAL));
                al->push_back(counter::ActionLimit(90,counter::GT,counter::MAJOR));
                al->push_back(counter::ActionLimit(80,counter::GT,counter::MINOR));
                al->push_back(counter::ActionLimit(70,counter::GT,counter::WARNING));
                o->setNewActions(al);
                tmgr->replaceObserver(*cname,o);
                tmgr->replaceTemplate(*cname,
                                      counter::CounterTemplate::create
                                      ("timesnapshot", o, 5, 100));
            }
        }
        {
            const char* cname = "sys.smpp.queue.global";
            counter::ActionTable* o = new counter::ActionTable();
            counter::ActionList* al = new counter::ActionList();
            al->push_back(counter::ActionLimit(95,counter::GT,counter::CRITICAL));
            al->push_back(counter::ActionLimit(90,counter::GT,counter::MAJOR));
            al->push_back(counter::ActionLimit(80,counter::GT,counter::MINOR));
            al->push_back(counter::ActionLimit(70,counter::GT,counter::WARNING));
            o->setNewActions(al);
            tmgr->replaceObserver(cname,o);
            tmgr->replaceTemplate( cname,
                                   counter::CounterTemplate::create
                                   ("accumulator",o));
        }
        {
            const char* names[] = { "sys.traffic.smpp.sme", "sys.traffic.smpp.smsc", 0 };
            for ( const char** cname = names; *cname != 0; ++cname ) {
                counter::ActionTable* o = new counter::ActionTable();
                counter::ActionList* al = new counter::ActionList();
                al->push_back(counter::ActionLimit(95,counter::GT,counter::CRITICAL));
                al->push_back(counter::ActionLimit(90,counter::GT,counter::MAJOR));
                al->push_back(counter::ActionLimit(80,counter::GT,counter::MINOR));
                al->push_back(counter::ActionLimit(70,counter::GT,counter::WARNING));
                o->setNewActions(al);
                tmgr->replaceObserver(*cname,o);
                tmgr->replaceTemplate(*cname,
                                      counter::CounterTemplate::create
                                      ("timesnapshot", o, 5, 100));
            }
        }
    }

    std::auto_ptr<pvss::core::client::Client> pvssClnt;
    try {
        smsc_log_info(log, "Personalization client initializing...");
        smsc_log_warn(log, "Personalization client initializing host=%s port=%d", cfg.getPersClientConfig().host.c_str(), cfg.getPersClientConfig().port);

        // --- configuration is taken from flooder
        pvss::core::client::ClientConfig clientConfig;
        try {
            // setting default values
            clientConfig.setHost("127.0.0.1");
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
            smsc_log_error(log, "cannot set default value: %s", e.what() );
            fprintf(stderr,"cannot set default value: %s\n", e.what() );
            abort();
        }

        smsc::util::config::Manager::init("config.xml");
        smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();
        ::readClientConfig(log, clientConfig, manager);

        pvssClnt.reset( new pvss::core::client::ClientCore
                        ( new pvss::core::client::ClientConfig(clientConfig),
                          new pvss::pvap::PvapProtocol ));

        pvssClnt->startup();

        // FIXME: init
        /*
        pc->init( pcfg.host.c_str(),
                  pcfg.port,
                  pcfg.timeout,
                  pcfg.pingTimeout,
                  pcfg.reconnectTimeout,
                  pcfg.maxCallsCount,
                  pcfg.connections,
                  pcfg.connPerThread );
         */
        pvssInited_ = true;
        smsc_log_info(log, "Personalization client initialized");
    } catch(std::exception& e) {
        throw Exception("Exception during initialization of PersClient: %s", e.what());
    }catch (...)
    {
        throw Exception("Exception during initialization of PersClient: unknown error");
    }


    //********************************************************
    try {
        LongCallManagerImpl* lcm = new LongCallManagerImpl(pvssClnt.release());
        lcm->init( cfg.getLongCallManConfig().maxThreads );
        lcmInited_ = true;
    }catch(...)
    {
        throw Exception("Exception during initialization of LongCallManager");
    }

    //********** Billing manager initialization ***********
    try {

        bill::BillingManagerImpl* bm = new bill::BillingManagerImpl;
        bm->init( cfg );
        billInited_ = true;
        bm->Start();

    }catch(...)
    {
        throw Exception("Exception during initialization of BillingManager");
    }


    //*********** SessionManager initialization **************
    SessionManagerImpl* sessman = 0;
    try{
        smsc_log_info(log, "Session Manager is being created..." );

        // it will auto-register
        sessman = new SessionManagerImpl;
        sessman->init( cfg.getSessionManConfig(), mynode, fsq );
        sessInited_ = true;
        // sessman->Start();

        smsc_log_info(log, "Session Manager is created" );
    }catch(exception& e){
      smsc_log_error(log, "Scag.init exception: %s", e.what());
      __warning__("Sessioan Manager is not created.");
        ::abort();
    }catch(...){
      __warning__("Session Manager is not created.");
        ::abort();
    }

    //********** Statistics manager initialization ***********
    try{

        stat::StatisticsManager* sm = new stat::StatisticsManager;
        sm->init( cfg.getStatManConfig() );
        statInited_ = true;
        smsc_log_info(log, "Statistics manager inited" );

    } catch(exception& e){
      smsc_log_warn(log, "Smsc.init exception: %s", e.what());
      __warning__("Statistics manager is not started.");
    }catch(...){
      __warning__("Statistics manager is not started.");
    }

    //************** RuleEngine initialization ***************
    try {
      smsc_log_info(log, "Rule Engine is starting..." );

      using scag::config::ConfigView;
      std::auto_ptr<ConfigView> cv(new ConfigView(*cfg.getConfig(),"RuleEngine"));

      auto_ptr <char> loc(cv->getString("location", 0, false));
      if (!loc.get()) throw Exception("RuleEngine.location not found");

        std::string location = loc.get();

        re::RuleEngineImpl* re = new re::RuleEngineImpl;
        re->init( location );
        // reInited_ = true;
        smsc_log_info(log, "Rule Engine started" );
    } catch (SCAGException& e) {
      smsc_log_warn(log, "%s", e.what());
      __warning__("Rule Engine is not started.");
    } catch(...){
      __warning__("Unknown error: rule Engine is not started.");
    }

    if ( sessman ) {
        sessman->Start();
    }

#ifdef SNMP
    //************** SNMP thread initialization **************
    try {
        const bool enabled = cfg.getConfig()->getBool("snmp.enabled");
        if ( enabled ) {
            const std::string socket = cfg.getConfig()->getString("snmp.socket");
            // if ( ! socket.empty() ) {
                smsc_log_info(log,"creating snmpwrapper @ '%s'", socket.c_str());
                snmp_.reset(new snmp::SnmpWrapper(socket));
                snmpthread_.reset(new snmp::SnmpTrapThread(snmp_.get()));
                snmpthread_->Start();
            // }
        }
    } catch (std::exception& e) {
        smsc_log_warn(log, "cannot initialize snmp: %s", e.what());
    } catch (...) {
        smsc_log_warn(log, "cannot initialize snmp: unknown exception" );
    }
#endif

    //************** SmppManager initialization **************

    scagHost=cfg.getConfig()->getString("smpp.host");
    scagPort=cfg.getConfig()->getInt("smpp.port");

    try {
        smsc_log_info(log, "Smpp Manager is starting");
        SmppManagerImpl* sm = new SmppManagerImpl
            (
#ifdef SNMP
             snmpthread_.get()
#else
             0
#endif
             );
        sm->Init( findConfigFile("../conf/smpp.xml") );
        smppInited_ = true;
        smsc_log_info(log, "Smpp Manager started");
    } catch(Exception& e) {
      throw Exception("Exception during initialization of SmppManager: %s", e.what());
    } catch (XMLException& e) {
        scag2::re::StrX msg(e.getMessage());
        throw Exception("Exception during initialization of SmppManager: %s", msg.localForm());
    } catch (...) {
      throw Exception("Exception during initialization of SmppManager: unknown error");
    }


    //************** HttpManager initialization **************
    try {
        smsc_log_info(log, "Http Manager is starting");

        // transport::http::HttpProcessor::Init("./conf");
        // transport::http::HttpProcessor& hp = transport::http::HttpProcessor::Instance();
        transport::http::HttpRouterImpl* rp = new transport::http::HttpRouterImpl;
        transport::http::HttpTraceRouter::setInstance( rp );
        rp->init( "./conf/http_routes__.xml" );

        transport::http::HttpProcessorImpl* hp = new transport::http::HttpProcessorImpl;
        hp->init( "./conf" );

        transport::http::HttpManagerImpl* mp = 
            new transport::http::HttpManagerImpl;
        mp->init( *hp, cfg.getHttpManConfig());
        httpInited_ = true;
        smsc_log_info(log, "Http Manager started");

    }catch(Exception& e)
    {
        throw Exception("Exception during initialization of HttpManager: %s", e.what());
    }catch (XMLException& e)
    {
        re::StrX msg(e.getMessage());

        throw Exception("Exception during initialization of HttpManager: %s", msg.localForm());
    }catch (...)
    {
        throw Exception("Exception during initialization of HttpManager: unknown error");
    }


    try{
        stat::StatisticsManager::Instance().Start();
        smsc_log_info(log, "Statistics manager started" );        
    }catch(exception& e){
      smsc_log_warn(log, "Smsc.init exception: %s", e.what());
      __warning__("Statistics manager is not started.");
    }catch(...){
      __warning__("Statistics manager is not started.");
    }
    
    smsc_log_info(log, "SCAG init complete\n\n" );
    // __trace__("Smsc::init completed");
}

void Scag::shutdown()
{
    // __trace__("shutting down");
    smsc_log_info( log, "SCAG is shutting down\n\n");
    if ( pvssInited_ ) {
        lcm::LongCallManager::Instance().pvssClient().shutdown();
        smsc_log_debug(log,"pvssclient notified");
    }
    if ( billInited_ ) {
        bill::BillingManager::Instance().Stop();     // to prevent dangling longcalls
        smsc_log_debug(log,"billman notified");
    }
    if ( lcmInited_ ) {
        lcm::LongCallManager::Instance().shutdown(); // to prevent dangling longcalls
        smsc_log_debug(log,"lcm notified");
    }
    if ( httpInited_ ) {
        transport::http::HttpManager::Instance().shutdown();
        smsc_log_debug(log,"httpman notified");
    }
    if ( smppInited_ ) {
        transport::smpp::SmppManager::Instance().shutdown();
        smsc_log_debug(log,"smppman notified");
    }
    if ( sessInited_ ) {
        sessions::SessionManager::Instance().Stop();
        smsc_log_debug(log,"sessman notified");
    }

    counter::Manager::getInstance().stop();
    smsc_log_debug(log,"counter manager stopped");

#ifdef SNMP
    if (snmpthread_.get()) snmpthread_->Stop();
#endif
    // stat::Statistics::Instance().Stop();
    smsc_log_info(log,"All components are stopped");
}

} //scag
