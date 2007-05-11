#include "scag/scag.h"

#include <memory>
#include "util/debug.h"
#include "core/synchronization/Event.hpp"
#include "util/Exception.hpp"
#include "logger/Logger.h"
#include "util/regexp/RegExp.hpp"
#include "util/config/ConfigView.h"
#include <typeinfo>
#include "util/findConfigFile.h"

#include "scag/config/ConfigManager.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/config/route/RouteStructures.h"
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
//#include "scag/admin/SCAGAdminCommand.h"

#include "scag/lcm/LongCallManager.h"

namespace scag
{
    using namespace scag::util::encodings;

    using namespace xercesc;

    using namespace scag::exceptions;
    using namespace scag::transport::smpp::router;
    using scag::config::RouteConfig;
    using scag::config::Route;
    using scag::config::Source;
    using scag::config::Destination;
    using scag::config::DestinationHash;
    using scag::config::SourceHash;
    using scag::config::Mask;
    using scag::config::MaskVector;
    using smsc::sms::Address;
    extern void loadRoutes(RouteManager* rm, const scag::config::RouteConfig& rc,bool traceit=false);

static inline void makeAddress_(Address& addr,const string& mask)
{
  addr=Address(mask.c_str());
}

void loadRoutes(RouteManager* rm,const scag::config::RouteConfig& rc,bool traceit)
{
  try
  {
    Route *route;
    for (RouteConfig::RouteIterator ri = rc.getRouteIterator();
         ri.fetchNext(route) == RouteConfig::success;)
    {
      char * dest_key;
      char* src_key;
      Source src;
      Destination dest;
      RouteInfo rinfo;
      for (DestinationHash::Iterator dest_it = route->getDestinations().getIterator();
           dest_it.Next(dest_key, dest);)
      {
        for (SourceHash::Iterator src_it = route->getSources().getIterator();
             src_it.Next(src_key, src);)
        {
          // masks
          if(dest.isSubject())
          {
            rinfo.dstSubj=( std::string("subj:")+dest.getIdString() ).c_str();
          }
          const MaskVector& dest_masks = dest.getMasks();
          for (MaskVector::const_iterator dest_mask_it = dest_masks.begin();
               dest_mask_it != dest_masks.end();
               ++dest_mask_it)
          {
            makeAddress_(rinfo.dest,*dest_mask_it);
            if(!dest.isSubject())
            {
              rinfo.dstSubj=( std::string("mask:")+*dest_mask_it ).c_str();
            }
            const MaskVector& src_masks = src.getMasks();
            if(src.isSubject())
            {
              rinfo.srcSubj=( std::string("subj:")+src.getIdString() ).c_str();
            }
            for(MaskVector::const_iterator src_mask_it = src_masks.begin();
                src_mask_it != src_masks.end();
                ++src_mask_it)
            {
              if(!src.isSubject())
              {
                rinfo.srcSubj=( std::string("mask:")+*src_mask_it ).c_str();
              }
              makeAddress_(rinfo.source,*src_mask_it);
              rinfo.smeSystemId = dest.getSmeIdString().c_str();//dest.smeId;
              rinfo.srcSmeSystemId = route->getSrcSmeSystemId().c_str();
//              __trace2__("sme sysid: %s",rinfo.smeSystemId.c_str());
              rinfo.archived=route->isArchiving();
              rinfo.enabled = route->isEnabling();
              rinfo.routeId=route->getId();
              rinfo.serviceId=route->getServiceId();

              try{
                rm->addRoute(rinfo);
              }
              catch(exception& e)
              {
                __warning2__("[route skiped] : %s",e.what());
              }
            }
          }
        }
      }
    }
    rm->commit(traceit);
  }
  catch(...)
  {
    rm->cancel();
    throw;
  }
}

using std::auto_ptr;
using std::string;
using std::exception;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;
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

    //smsc::util::regexp::RegExp::InitLocale();

    /* try {
        InitLicense(*cfg.getLicConfig());
    } catch (exception& e) {
        smsc_log_error(log, "Cannot initialize license. Initialization stopped. %s", e.what());
        throw;
    }*/
    //********************************************************
    
    try {
        LongCallManager::Init(cfg.getLongCallManConfig());
    }catch(...)
    {
        throw Exception("Exception during initialization of LongCallManager");
    }

    //********************************************************
    //********** Statistics manager initialization ***********
    try{
      StatisticsManager::init(cfg.getStatManConfig());

      smsc_log_info(log, "Statistics manager started" );
    }catch(exception& e){
      smsc_log_warn(log, "Smsc.init exception: %s", e.what());
      __warning__("Statistics manager is not started.");
    }catch(...){
      __warning__("Statistics manager is not started.");
    }
    //********************************************************


    //********************************************************
    //********** Billing manager initialization ***********
    try {
        BillingManager::Init(cfg.getBillManConfig());
    }catch(...)
    {
        throw Exception("Exception during initialization of BillingManager");
    }


    //********************************************************
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
    //********************************************************


    //********************************************************
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
    //********************************************************

    //********************************************************
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

    //********************************************************

    scagHost=cfg.getConfig()->getString("smpp.host");
    scagPort=cfg.getConfig()->getInt("smpp.port");

    //************** SmppManager initialization **************
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
    //********************************************************

  /*
  ////////////////////////// FOR TEST

  scag::sessions::CSessionKey key;

  SMS sms1, sms2, sms3;
  //smsc::smpp::UssdServiceOpValue::PSSR_INDICATION == sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)
  //sms1.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, smsc::smpp::UssdServiceOpValue::PSSR_INDICATION);
  sms2.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, 100);


  std::string text;
  const char * inBuff = "2 3 LA";

  Convertor::UTF8ToGSM7Bit(inBuff, strlen(inBuff), text);
  sms1.setIntProperty(Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::SMSC7BIT);
  sms1.setBinProperty(Tag::SMPP_SHORT_MESSAGE, text.data(), text.size());

  std::string data;
  uint16_t tag;
  uint16_t val_len;
  char val = 1;

  tag = 0x4901;
  val_len = 1;
  data.append((char *)&tag, 2);
  data.append((char *)&val_len, 2);
  data.append(&val, 1);


  tag = 0x4903;
  val_len = 4;

  data.append((char *)&tag, 2);
  data.append((char *)&val_len, 2);
  data.append("lala",4);


  sms3.setBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS, data.data(), data.size());

  Address abonent("+79137354153");
  sms1.originatingAddress = abonent;
  sms2.destinationAddress = abonent;
  sms3.originatingAddress = abonent;
  sms3.destinationAddress = abonent;


  unsigned short shbuff[100];
  shbuff[0] = 0x4903;
  shbuff[1] = 2;
  shbuff[2] = 12593;

  sms2.setBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS, (const char *)shbuff, 6);

  char buff[128];
  scag::transport::smpp::SmppCommand commandDeliver1 = scag::transport::smpp::SmppCommand::makeDeliverySm(sms1,1);
  scag::transport::smpp::SmppCommand commandDeliverResp = scag::transport::smpp::SmppCommand::makeDeliverySmResp(buff,1,0);
  scag::transport::smpp::SmppCommand commandSubmit = scag::transport::smpp::SmppCommand::makeSubmitSm(sms2,1);
  scag::transport::smpp::SmppCommand commandSubmitResp = scag::transport::smpp::SmppCommand::makeSubmitSmResp(buff,1,0, true);
  scag::transport::smpp::SmppCommand commandPureDataSmResp = scag::transport::smpp::SmppCommand::makeDataSmResp(buff,1,0);

  scag::transport::smpp::SmppCommand commandDeliver2 = scag::transport::smpp::SmppCommand::makeDeliverySm(sms3,1);


  scag::transport::smpp::SmppCommand commandPureSubmit = scag::transport::smpp::SmppCommand::makeSubmitSm(sms3,1);
  scag::transport::smpp::SmppCommand commandPureDataSm = scag::transport::smpp::SmppCommand::makeSubmitSm(sms3,1);

  commandPureDataSm->cmdid = scag::transport::smpp::DATASM;
  commandPureDataSm->get_smsCommand().dir = scag::transport::smpp::dsdSrv2Sc;
  commandPureDataSm.setServiceId(1);



  //commandSubmitResp->get_resp()->set_sms(&sms2);


  commandPureSubmit.setServiceId(1);


  commandDeliver1.setServiceId(1);
  commandDeliver2.setServiceId(1);

  commandDeliverResp.setServiceId(1);
  commandDeliverResp.setOperationId(1);

  commandSubmit.setServiceId(1);
  commandSubmitResp.setServiceId(1);
  commandSubmitResp.setOperationId(1);

  commandPureDataSmResp.setServiceId(1);
  commandPureDataSmResp.setOperationId(1);


  SMS * pSms = new SMS();
  (*pSms) = sms3;

  commandDeliverResp->get_resp()->set_sms(pSms);
  commandPureDataSmResp->get_resp()->set_sms(pSms);
  commandSubmitResp->get_resp()->set_sms(pSms);

  commandPureDataSmResp->get_resp()->set_dir(scag::transport::smpp::dsdSrv2Sc);




  SessionManager& sm = SessionManager::Instance();
  scag::sessions::SessionPtr sessionPtr = sm.newSession(key);
  scag::sessions::Session * session = sessionPtr.Get();

  if (session) smsc_log_warn(log, "SESSION IS VALID");
  
  
  scag::re::RuleEngine::Instance().process(commandPureDataSm, *session);
  sm.releaseSession(sessionPtr);

  sessionPtr = sm.getSession(key);
  session = sessionPtr.Get();

  scag::re::RuleEngine::Instance().process(commandPureDataSmResp, *session);
  sm.releaseSession(sessionPtr);
  
  
  /*
  scag::re::RuleEngine::Instance().process(commandDeliver1, *session);
  sm.releaseSession(sessionPtr);
  
  
  sessionPtr = sm.getSession(key);
  session = sessionPtr.Get();
  scag::re::RuleEngine::Instance().process(commandDeliverResp, *session);
  sm.releaseSession(sessionPtr);
  */
  
  //sessionPtr = sm.getSession(key);
  //session = sessionPtr.Get();

  /*
  scag::re::RuleEngine::Instance().process(commandPureSubmit, *session);
  sm.releaseSession(sessionPtr);


  sessionPtr = sm.getSession(key);
  session = sessionPtr.Get();
  scag::re::RuleEngine::Instance().process(commandSubmitResp, *session);
  sm.releaseSession(sessionPtr);
  */
  /*
  //PURE SUBMIT
  sessionPtr = sm.getSession(key);
  session = sessionPtr.Get();
  scag::re::RuleEngine::Instance().process(commandPureSubmit, *session);
  sm.releaseSession(sessionPtr);
  //PURE SUBMIT


  sessionPtr = sm.getSession(key);
  session = sessionPtr.Get();
  scag::re::RuleEngine::Instance().process(commandDeliver2, *session);
  sm.releaseSession(sessionPtr);
   */
  ////////////////////////// FOR TEST


/*
  scag::admin::CommandListSmsc * commandListSmsc = new scag::admin::CommandListSmsc(0);
  commandListSmsc->CreateResponse(this);
  */


    //********************************************************
    //************** HttpManager initialization **************
    try {
        smsc_log_info(log, "Http Manager is starting");

//        scag::transport::http::httpLogger = smsc::logger::Logger::getInstance("httpLogger");

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
  //********************************************************


  smsc_log_info(log, "SCAG init complete" );

  __trace__("Smsc::init completed");
}

/*void Scag::run()
{
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.run");
*/
  //TODO: report performance on Speed Monitor
    /*SpeedMonitor *sm=new SpeedMonitor(&perfDataDisp,this);
    FILE *f=fopen("stats.txt","rt");
    if(f)
    {
      time_t ut;
      fscanf(f,"%d %lld %lld %lld %lld %lld %lld",
        &ut,
        &acceptedCounter,
        &rejectedCounter,
        &deliveredCounter,
        &deliverErrCounter,
        &transOkCounter,
        &transFailCounter
      );
      startTime=time(NULL)-ut;
      sm->setStartTime(startTime);
      fclose(f);
      remove("stats.txt");
    }
    sm->run();*/
/*  while(!stopFlag)
  {
      sleep(1);
  }
    smsc_log_info(log, "Scag stoped" );
}
*/
void Scag::shutdown()
{
  __trace__("shutting down");

  scag::transport::http::HttpManager::Instance().shutdown();
  scag::transport::smpp::SmppManager::shutdown();
  //tp.shutdown();
  //tp2.shutdown();
  LongCallManager::shutdown();  
}

void Scag::reloadTestRoutes(const RouteConfig& rcfg)
{
  auto_ptr<RouteManager> router(new RouteManager());
  //router->assign(&smeman);
  loadRoutes(router.get(),rcfg,true);
  ResetTestRouteManager(router.release());
}

} //scag
