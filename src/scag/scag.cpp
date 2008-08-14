#include "scag/scag.h"

#include <memory>
#include "util/debug.h"
#include "core/synchronization/Event.hpp"
#include "util/Exception.hpp"
#include "logger/Logger.h"
#include "util/regexp/RegExp.hpp"
#include "scag/config/base/ConfigView.h"
#include <typeinfo>
#include "util/findConfigFile.h"

#include "scag/transport/smpp/router/load_routes.h"
#include "scag/config/ConfigManager.h"
#include "scag/bill/BillingManager.h"
#include "scag/stat/StatisticsManager.h"
#include "scag/sessions/SessionManager.h"

#include "util/findConfigFile.h"
#include "scag/re/RuleEngine.h"

#include "util/xml/utilFunctions.h"
#include "scag/re/XMLHandlers.h"

#include "scag/transport/http/HttpRouter.h"
#include "scag/pers/PersClient.h"

#include "scag/util/encodings/Encodings.h"

#include "scag/lcm/LongCallManager.h"

namespace scag{

using namespace scag::exceptions;
using std::auto_ptr;
using std::string;
using std::exception;
using namespace smsc::sms;
using namespace smsc::core::synchronization;
using smsc::util::Exception;
using scag::config::ConfigManager;
using scag::bill::BillingManager;
using scag::stat::StatisticsManager;
using scag::config::BillingManagerConfig;
using scag::sessions::SessionManager;
using scag::lcm::LongCallManager;
using smsc::util::findConfigFile;

Scag::~Scag()
{
}

void Scag::init()
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

      SessionManager::Init(cfg.getSessionManConfig());

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

      scag::re::RuleEngine & re = scag::re::RuleEngine::Instance();
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
      scag::transport::smpp::SmppManager::Init(findConfigFile("../conf/smpp.xml"));
      smsc_log_info(log, "Smpp Manager started");
    } catch(Exception& e) {
      throw Exception("Exception during initialization of SmppManager: %s", e.what());
    } catch (XMLException& e) {
      scag::re::StrX msg(e.getMessage());
      throw Exception("Exception during initialization of SmppManager: %s", msg.localForm());
    } catch (...) {
      throw Exception("Exception during initialization of SmppManager: unknown error");
    }

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
  scag::transport::http::HttpManager::Instance().shutdown();
  scag::transport::smpp::SmppManager::shutdown();
  LongCallManager::shutdown();  
  scag::pers::client::PersClient::Instance().Stop();  
  scag::bill::BillingManager::Instance().Stop();  
  scag::stat::StatisticsManager::Instance().Stop();   
}

} //scag
