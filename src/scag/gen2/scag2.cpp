#include <memory>
#include <typeinfo>

#include "core/synchronization/Event.hpp"
#include "logger/Logger.h"
#include "scag/bill/impl/BillingManager.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/config/base/ConfigView.h"
#include "scag/lcm/impl/LongCallManagerImpl.h"
#include "scag/pers/util/PersClient2.h"
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


namespace {

using namespace scag2;
using namespace scag2::transport;
using namespace scag2::transport::smpp;

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

        default : {
            throw std::runtime_error("transports other than SMPP is not implemented yet");
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

Scag::~Scag()
{
}

void Scag::init( unsigned mynode )
{
    log = smsc::logger::Logger::getInstance("scag.init");

    smsc_log_info(log, "SCAG start initialisation...");

    ConfigManager& cfg = ConfigManager::Instance();

    //********************************************************
    try {
        LongCallManagerImpl* lcm = new LongCallManagerImpl();
        lcm->init( cfg.getLongCallManConfig().maxThreads );
    }catch(...)
    {
        throw Exception("Exception during initialization of LongCallManager");
    }

    //********** Billing manager initialization ***********
    try {

        bill::BillingManagerImpl* bm = new bill::BillingManagerImpl;
        bm->init( cfg.getBillManConfig() );
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
        smsc_log_info(log, "Statistics manager inited" );

    } catch(exception& e){
      smsc_log_warn(log, "Smsc.init exception: %s", e.what());
      __warning__("Statistics manager is not started.");
    }catch(...){
      __warning__("Statistics manager is not started.");
    }

    //************** Personalization client initialization **************

    try {
        smsc_log_info(log, "Personalization client initializing...");
        smsc_log_warn(log, "Personalization client initializing host=%s port=%d", cfg.getPersClientConfig().host.c_str(), cfg.getPersClientConfig().port);

        pers::util::PersClient::Init(cfg.getPersClientConfig());

        smsc_log_info(log, "Personalization client initialized");
    } catch(std::exception& e) {
        throw Exception("Exception during initialization of PersClient: %s", e.what());
    }catch (...)
    {
        throw Exception("Exception during initialization of PersClient: unknown error");
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

    //************** SmppManager initialization **************

    scagHost=cfg.getConfig()->getString("smpp.host");
    scagPort=cfg.getConfig()->getInt("smpp.port");

    try {
        smsc_log_info(log, "Smpp Manager is starting");
        SmppManagerImpl* sm = new SmppManagerImpl();
        sm->Init( findConfigFile("../conf/smpp.xml") );
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
    pers::util::PersClient::Instance().Stop();   // to prevent dangling longcalls
    lcm::LongCallManager::Instance().shutdown(); // to prevent dangling longcalls
    transport::http::HttpManager::Instance().shutdown();
    transport::smpp::SmppManager::Instance().shutdown();
    sessions::SessionManager::Instance().Stop();
    // bill::BillingManager::Instance().Stop();
    // stat::Statistics::Instance().Stop();
}

} //scag
