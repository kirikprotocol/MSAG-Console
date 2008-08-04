#include <memory>
#include <typeinfo>

#include "core/synchronization/Event.hpp"
#include "logger/Logger.h"
#include "scag/bill/BillingManager.h"
#include "scag/config/ConfigManager2.h"
#include "scag/config/ConfigView.h"
#include "scag/lcm/LongCallManager2.h"
#include "scag/pers/PersClient.h"
#include "scag/re/RuleEngine2.h"
#include "scag/re/XMLHandlers2.h"
#include "scag/scag2.h"
#include "scag/sessions/SessionManager2.h"
#include "scag/stat/StatisticsManager.h"
// #include "scag/transport/http/HttpRouter.h"
#include "scag/transport/smpp/router/load_routes.h"
#include "scag/util/encodings/Encodings.h"
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
            if ( action == RESERVE ) return 1;

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
    virtual SCAGCommand* popCommand() {
        return 0;
    }

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
using scag2::config::ConfigManager;
using scag::bill::BillingManager;
using scag::stat::StatisticsManager;
using scag::config::BillingManagerConfig;
using scag2::sessions::SessionManager;
using scag2::lcm::LongCallManager;
using smsc::util::findConfigFile;

Scag::~Scag()
{
}

void Scag::init( unsigned mynode )
{
    smsc::logger::Logger *log=smsc::logger::Logger::getInstance("scag.init");


    smsc_log_info(log, "SCAG start initialisation...");


    ConfigManager & cfg = ConfigManager::Instance();

    //********************************************************
    try {
        LongCallManager::Init(cfg.getLongCallManConfig());
    }catch(...)
    {
        throw Exception("Exception during initialization of LongCallManager");
    }

    //********** Statistics manager initialization ***********
    try{
      StatisticsManager::init(cfg.getStatManConfig());

      smsc_log_info(log, "Statistics manager inited" );
    }catch(exception& e){
      smsc_log_warn(log, "Smsc.init exception: %s", e.what());
      __warning__("Statistics manager is not started.");
    }catch(...){
      __warning__("Statistics manager is not started.");
    }

    //********** Billing manager initialization ***********
    try {
        BillingManager::Init(cfg.getBillManConfig());
    }catch(...)
    {
        throw Exception("Exception during initialization of BillingManager");
    }


    //*********** SessionManager initialization **************
    try{
      smsc_log_info(log, "Session Manager is starting..." );

      SessionManager::Init(mynode, cfg.getSessionManConfig(), fsq);

      smsc_log_info(log, "Session Manager started" );
    }catch(exception& e){
      smsc_log_warn(log, "Scag.init exception: %s", e.what());
      __warning__("Sessioan Manager is not started.");
    }catch(...){
      __warning__("Session Manager is not started.");
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

      scag2::re::RuleEngine & re = scag2::re::RuleEngine::Instance();
      re.Init(location);

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
        StatisticsManager::Instance().Start();
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
  LongCallManager::shutdown();  
  scag::pers::client::PersClient::Instance().Stop();
  scag::bill::BillingManager::Instance().Stop();  
  scag::stat::StatisticsManager::Instance().Stop();   
}

} //scag
