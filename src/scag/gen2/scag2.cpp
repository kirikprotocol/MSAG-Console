#include <memory>
#include <typeinfo>

// #include "scag/transport/http/HttpRouter.h"

#include "core/synchronization/Event.hpp"
#include "logger/Logger.h"
#include "scag/bill/impl/BillingManager.h"
#include "scag/config/base/ConfigView.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/lcm/impl/LongCallManagerImpl.h"
#include "scag/pers/PersClient.h"
#include "scag/re/impl/RuleEngine2.h"
#include "scag/re/base/XMLHandlers2.h" // for StrX
#include "scag/sessions/impl/SessionManager2.h"
#include "scag/stat/impl/StatisticsManager.h"
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
        std::auto_ptr< SCAGCommand > c(cmd);
        switch ( cmd->getType() ) {
        case SMPP: {
            /// FIXME: use smppmanager.lcmprocessingcount to make sure command is processed before shutdown
            if ( action == RESERVE ) {
                c.release();
                return 1;
            }

            SmppManager::Instance().pushCommand( static_cast<SmppCommand*>(c.release()) );
            return 1;
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
    smsc::logger::Logger *log=smsc::logger::Logger::getInstance("scag.init");


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

    }catch(...)
    {
        throw Exception("Exception during initialization of BillingManager");
    }


    //*********** SessionManager initialization **************
    try{
        smsc_log_info(log, "Session Manager is starting..." );

        // it will auto-register
        SessionManagerImpl* sm = new SessionManagerImpl;
        sm->init( cfg.getSessionManConfig(), mynode, fsq );
        sm->Start();

        smsc_log_info(log, "Session Manager started" );
    }catch(exception& e){
      smsc_log_warn(log, "Scag.init exception: %s", e.what());
      __warning__("Sessioan Manager is not started.");
    }catch(...){
      __warning__("Session Manager is not started.");
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

        scag::pers::client::PersClient::Init(cfg.getPersClientConfig());

        smsc_log_info(log, "Personalization client initialized");
    }catch(Exception& e)
    {
        throw Exception("Exception during initialization of PersClient: %s", e.what());
    }catch (scag::pers::client::PersClientException& e)
    {
        smsc_log_error(log, "Exception during initialization of PersClient: %s", e.what());
//        throw Exception("Exception during initialization of PersClient: %s", e.what());
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

    //************** SmppManager initialization **************

    scagHost=cfg.getConfig()->getString("smpp.host");
    scagPort=cfg.getConfig()->getInt("smpp.port");

    try {
      smsc_log_info(log, "Smpp Manager is starting");
      transport::smpp::SmppManager::Init(findConfigFile("../conf/smpp.xml"));
      smsc_log_info(log, "Smpp Manager started");
    } catch(Exception& e) {
      throw Exception("Exception during initialization of SmppManager: %s", e.what());
    } catch (XMLException& e) {
        scag2::re::StrX msg(e.getMessage());
        throw Exception("Exception during initialization of SmppManager: %s", msg.localForm());
    } catch (...) {
      throw Exception("Exception during initialization of SmppManager: unknown error");
    }

#if 0
    //************** HttpManager initialization **************
    try {
        smsc_log_info(log, "Http Manager is starting");

        scag::transport::http::HttpProcessor::Init("./conf");
        scag::transport::http::HttpProcessor& hp = scag::transport::http::HttpProcessor::Instance();

        scag::transport::http::HttpTraceRouter::Init("./conf/http_routes__.xml");

        scag::transport::http::HttpManager::Init(hp, cfg.getHttpManConfig());

        smsc_log_info(log, "Http Manager started");
    }catch(Exception& e)
    {
        throw Exception("Exception during initialization of HttpManager: %s", e.what());
    }catch (XMLException& e)
    {
        scag::re::StrX msg(e.getMessage());

        throw Exception("Exception during initialization of HttpManager: %s", msg.localForm());
    }catch (...)
    {
        throw Exception("Exception during initialization of HttpManager: unknown error");
    }
#endif

    try{
        stat::StatisticsManager::Instance().Start();
        smsc_log_info(log, "Statistics manager started" );        
    }catch(exception& e){
      smsc_log_warn(log, "Smsc.init exception: %s", e.what());
      __warning__("Statistics manager is not started.");
    }catch(...){
      __warning__("Statistics manager is not started.");
    }
    
  smsc_log_info(log, "SCAG init complete" );

  __trace__("Smsc::init completed");
}

void Scag::shutdown()
{
  __trace__("shutting down");
//   scag::transport::http::HttpManager::Instance().shutdown();
  scag2::transport::smpp::SmppManager::shutdown();
    lcm::LongCallManager::Instance().shutdown();  
  scag::pers::client::PersClient::Instance().Stop();
    bill::BillingManager::Instance().Stop();
    stat::Statistics::Instance().Stop();
}

} //scag
