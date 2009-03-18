/*
    SibInfoConsalting
    Rule Engine teest 3
    created by Green: green@sibinco.ru
    Javascript engine based
*/
#define TESTING


#include <core/threads/Thread.hpp>
#include <scag/re/RuleEngine.h>
#include <logger/Logger.h>
#include <scag/re/RuleStatus.h>
#include <scag/transport/smpp/SmppCommand.h>

#include <scag/sessions/SessionManager.h>
#include <scag/sessions/SessionStore.h>
#include <scag/sessions/Session.h>

#include <scag/bill/BillingManager.h>
#include <scag/config/ConfigManager.h>
#include <sms/sms.h>
#include <scag/transport/SCAGCommand.h>
#include <scag/exc/SCAGExceptions.h>
#include "scag/config/route/RouteConfig.h"
#include "scag/config/route/RouteStructures.h"

#include <scag/stat/StatisticsManager.h>
#include "scag/pers/PersClient.h"
#include <scag/transport/smpp/router/route_manager.h>
//inthash
#include <core/buffers/IntHash.hpp>

#include <stddef.h>
#include <fstream>
#include <time.h>
#include <thread.h>
#include <stdio.h>
#include <stdlib.h>
#include "commandgen.h"
#include <unistd.h>

using scag::re::RuleEngine;
using scag::re::RuleStatus;
using std::exception;

using scag::sessions::SessionManagerConfig;
using scag::sessions::CSessionKey;
using scag::sessions::SessionPtr;
using scag::transport::smpp::SmppCommand;

using namespace scag::sessions;
using namespace scag::config;
using namespace scag::exceptions;

using namespace scag::transport::smpp::router;
using scag::config::RouteConfig;
using scag::config::Route;

using namespace scag::transport::smpp;
using namespace smsc::sms;
using namespace smsc::logger;
using namespace scag::util;
using namespace scag::stat;

using scag::bill::BillingManager;
using scag::stat::StatisticsManager;
using scag::config::BillingManagerConfig;
//inthash
using namespace smsc::core::buffers;

extern bool stopProcess;
smsc::logger::Logger *logger;

static FILE*pFile;

RuleEngine * engine=0;
SessionManager * smanager=0;
SessionManagerConfig cfg;   

SmppCommand command;


/*********************************************/
/* routines for RuleEngine interface working */
/*                              */ 
/*                                           */
/*                                           */
/*********************************************/

/**Create rules from directory */

/*int  initReInstance( std::string& dir_name)
{

// try{

     smsc_log_debug(logger,"RuleEngine::Init('%s')",dir_name.c_str());

     RuleEngine::Init(dir_name);
     RuleEngine& re2=RuleEngine::Instance();
         engine=&re2;
    if(engine==0)
    {
     smsc_log_error(logger,"RE == 0 !");
     return 0;
    }
    }
    catch(...)
    {

   	smsc_log_error(logger,"can't init RE instance !");

    }

  if(!engine)
  return 0;
return 1;
}
int  initSessionManagerInstance(std::string &dir_name,time_t tmint)
{

try{


  cfg.expireInterval=tmint;
         cfg.dir=dir_name;
  SessionManager::Init(cfg);
  SessionManager& sm2=SessionManager::Instance();
         smanager=&sm2;
 
  if(smanager==0)
   {
        smsc_log_error(logger," SessionManager==0  !");
	return 0;

   }
  
   }
   catch(...)
   {
      smsc_log_error(logger,"cant create SessionManager  !");
      return 0;

   } 
    smsc_log_debug(logger,"SessionManager Init success, cfg=%s, timeout=%d",dir_name.c_str(),tmint);
     
   return 1;
}                      */

/*static JSBool _initBillInstance(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 scag::bill::BillingManagerConfig bcfg;
 scag::bill::BillingManager::Init(bcfg);
 scag::config::ConfigManager::Init();

  return JS_TRUE;
}


static JSBool _updaterule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
     int ruleId=JSVAL_TO_INT(argv[0]);
     engine->updateRule(ruleId);
 
 return JS_TRUE;
}  

static JSBool _deleterule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
     int ruleId=JSVAL_TO_INT(argv[0]);

     engine->removeRule(ruleId);
     *rval=BOOLEAN_TO_JSVAL(true);
 
 return JS_TRUE;
}
*/

/*
    RuleRun(String strTransport,String strSCAGCommandID)
    generate SCAG command by name

 std::string cmd_name,     
 std::string oa,
       uint8_t oa_tn,
       uint8_t oa_np,
 std::string str_da,
       uint8_t da_tn,
       uint8_t da_np,
       uint32_t dialogid);
   
*/


///////////////////////////////////////////////////

int ruleRun(   std::string cmd_name,  
		std::string str_oa,  
  		uint8_t oa_tn   ,
  		uint8_t oa_np   ,
  		std::string str_da,
  		uint8_t da_tn    ,
  		uint8_t da_np    ,
  		uint32_t dialogid,
		int bdump,
		uint32_t ruleid,scag::sessions::SessionPtr& session)
{
 CSessionKey key;

  smsc_log_debug(logger,"RuleRun(%s,%s,%d,%d,%s,%d,%d,%d)",
    cmd_name.c_str(),
    str_oa.c_str(),
    oa_tn,
    oa_np,
    str_da.c_str(),
    da_tn,
    da_np,
    dialogid);

 scag::transport::smpp::SmppCommand cmd;

 String2SCAGCommang(&cmd,cmd_name,str_oa,oa_tn,oa_np,str_da,da_tn,da_np,dialogid);
 

  smsc::sms::Address oa,da ;

   oa.setNumberingPlan(oa_np);
   oa.setTypeOfNumber(oa_tn);
   oa.setValue(str_oa.length(),str_oa.c_str());
  
 key.abonentAddr = oa;
 cmd->set_serviceId(ruleid);
 cmd->set_operationId(1);

 if(session.Get()==0)
	session =  smanager->newSession(key);;//smanager->getSession(cmd);
 
  
     if (!session.Get()) 
     { 
       smsc_log_error(logger,"RuleRun():getSession('%s')==0 !",cmd_name.c_str());
       return 0;
     }

    try
      {
       smsc_log_debug(logger,"getSession for OA='%s'ok",str_oa.c_str());

       RuleStatus rs = engine->process(cmd,*session.Get());
       char buff[128];
       snprintf(buff,sizeof(buff),"cmd:%s %s%d",cmd_name.c_str(),"result = ",rs.result);
       smsc_log_debug(logger,buff);

	
	 if(bdump)
	  {
	      for(int i=0;i<SMS_LAST_TAG;i++)
      		{
		    if(Tag::tagNames[i]) 
		        {
			   switch(Tag::tagTypes[i]) 
				   {
				     case SMS_INT_TAG:
					      smsc_log_debug(logger,"%s :int: %d",Tag::tagNames[i],cmd->get_sms()->getIntProperty(i|((SMS_INT_TAG)<<8))); 
					    break;
				     case SMS_BIN_TAG:
					      //smsc_log_debug(logger,"%s type : %d",Tag::tagNames[i],); 
					    break;
				     case SMS_STR_TAG:  
					      smsc_log_debug(logger,"%s :string: '%s'",Tag::tagNames[i],cmd->get_sms()->getStrProperty(i|((SMS_STR_TAG)<<8)).c_str()); 
					    break;

				   }
			   }
	       }
   	}
	
	smsc_log_debug(logger," SMPP_SAR_TOTAL_SEGMENTS=%d SMPP_SAR_SEGMENT_SEQNUM)=%d", cmd->get_sms()->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS),	  cmd->get_sms()->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM));
	    
	
	/*_SmppCommand& _cmd = *cmd.operator ->();
	smsc_log_debug(logger,"operation id == %d",cmd.getOperationId());
	_cmd.get_resp()->set_sms(0);	
	*/
       //smanager->releaseSession(session);

      }
      catch (Exception& e)
      {
              smsc_log_error(logger,"cmd:%s:",cmd_name.c_str());
              smsc_log_error(logger,std::string("Process aborted: ") + e.what());
       
       return 0;
      }
  return 1;

}  
/**********************************************/  

IntHash <SessionPtr> sessions_list;

void sessionsOpen(int scnt)
{
 CSessionKey key;
 char number[20];
 
 for(int i=0;i<scnt;i++)
 {
	 smsc::sms::Address oa;
	 sprintf(number,"8%.7d",i);
     oa.setNumberingPlan(0);//numberingplan
     oa.setTypeOfNumber(1);//typeofnumber
     oa.setValue(8,number);

	 key.abonentAddr = oa; 
     sessions_list.Insert(i,smanager->newSession(key));
	
 }
}

void sessionsClose(int scnt)
{
 for(int i=0;i<scnt;i++)
 {

	 if(sessions_list.GetPtr(i))
	 {
		 if(sessions_list.Get(i).Get())
		 {
		 	smanager->releaseSession(sessions_list.Get(i));
		 }
		 else
		 {
			smsc_log_error(logger,"Error sessions for %d session is 0",i);
		 }
		 
		 sessions_list.Delete(i);
	 }
	 else
	 {
		 smsc_log_error(logger,"Error sessions wrong %d",i);
	 }
        
	
 }
}

void sessionsDeliverSM(int scnt)
{
	char number[20];

	for(int i=0;i<scnt;i++)
	{
	if(sessions_list.GetPtr(i))
	 {
		 
		sprintf(number,"8%.7d",i);
		std::string num = number;
		
		if(sessions_list.Get(i).Get())
		{
			ruleRun("deliver_sm",num,0,1,"897654326",0,1,51,0,1,sessions_list.Get(i));
		}
		else
		{
			smsc_log_error(logger,"sessionsDeliverSM:Error getting sessions %d session is 0",i);
		}

	 }
	 else
	 {
		 smsc_log_error(logger,"sessionsDeliverSM:Error sessions wrong %d",i);
	 }
	
	}
}

void init()
{
   try{
   	 smsc_log_info(logger,  "SCAG configuration loading..." );
	 scag::config::ConfigManager::Init();
	 smsc_log_info(logger,  "SCAG configuration is loaded" );

	}
       catch (scag::config::ConfigException &c)
       {
	 smsc_log_error(logger,"%s",c.what());	
	}
                                                
	ConfigManager & cfg = ConfigManager::Instance();

/********************************************************/
  try {
        BillingManager::Init(cfg.getBillManConfig());
    }catch(...)
    {
        throw Exception("Exception during initialization of BillingManager");
    }

/*********************************************************/
   try {
        smsc_log_info(logger, "Personalization client initializing...");

        using scag::config::ConfigView;

        std::auto_ptr<ConfigView> cv(new ConfigView(*cfg.getConfig(), "Personalization"));

        auto_ptr <char> host(cv->getString("host", NULL, false));

        if(!host.get())
            throw Exception("Empty host");

        scag::pers::client::PersClient::Init(host.get(), cv->getInt("port", NULL), cv->getInt("timeout", NULL));

        smsc_log_info(logger, "Personalization client initialized");
    }catch(Exception& e)
    {
        throw Exception("Exception during initialization of PersClient: %s", e.what());
    }catch (scag::pers::client::PersClientException& e)
    {
        smsc_log_error(logger, "Exception during initialization of PersClient: %s", e.what());
//        throw Exception("Exception during initialization of PersClient: %s", e.what());
    }catch (...)
    {
        throw Exception("Exception during initialization of PersClient: unknown error");
    }

//************** RuleEngine initialization ***************
  try {
      smsc_log_info(logger, "Rule Engine is starting..." );

      using scag::config::ConfigView;
      std::auto_ptr<ConfigView> cv(new ConfigView(*cfg.getConfig(),"RuleEngine"));

      auto_ptr <char> loc(cv->getString("location", 0, false));
      if (!loc.get()) throw Exception("RuleEngine.location not found");

      std::string location = loc.get();

      scag::re::RuleEngine & re = scag::re::RuleEngine::Instance();
      re.Init(location);

      engine=&re;
      smsc_log_info(logger, "Rule Engine started" );

  } catch (SCAGException& e) {
      smsc_log_warn(logger, "%s", e.what());
      __warning__("Rule Engine is not started.");
  } catch(...){
      __warning__("Unknown error: rule Engine is not started.");
  }

  //*********** SessionManager initialization **************
  try{
      smsc_log_info(logger, "Session Manager is starting..." );

      SessionManager::Init(cfg.getSessionManConfig());
      SessionManager& sm2=SessionManager::Instance();
      smanager=&sm2;
      smsc_log_info(logger, "Session Manager started" );
  }
  catch(exception& e)
  {
      smsc_log_warn(logger, "Scag.init exception: %s", e.what());
      __warning__("Sessioan Manager is not started.");
  }catch(...){
      __warning__("Session Manager is not started.");
  }
  //********************************************************

 //********** Statistics manager initialization ***********
  try{
      StatisticsManager::init(cfg.getStatManConfig());

      smsc_log_info(logger, "Statistics manager started" );
  }catch(exception& e){
      smsc_log_warn(logger, "init exception: %s", e.what());
      __warning__("Statistics manager is not started.");
  }catch(...){
      __warning__("Statistics manager is not started.");
  }

}

void fortest(std::string str_oa,std::string str_da)
{
  scag::sessions::CSessionKey key;

  SMS sms1;
 smsc::sms::Address oa,da ;


 oa.setNumberingPlan(1);
 oa.setTypeOfNumber(1);

 da.setNumberingPlan(1);
 da.setTypeOfNumber(1);

 oa.setValue(str_oa.length(),str_oa.c_str());
 da.setValue(str_da.length(),str_da.c_str());

 sms1.setDestinationAddress(da);
 sms1.setOriginatingAddress(oa);

  //smsc::smpp::UssdServiceOpValue::PSSR_INDICATION == sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)
  //sms1.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, smsc::smpp::UssdServiceOpValue::PSSR_INDICATION);
  //sms2.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, 100);

  char buff[128];
  scag::transport::smpp::SmppCommand commandDeliver1 = scag::transport::smpp::SmppCommand::makeDeliverySm(sms1,1);
  commandDeliver1.setServiceId(1);

  SessionManager& sm = SessionManager::Instance();
  key.abonentAddr = oa;
  scag::sessions::SessionPtr sessionPtr = sm.newSession(key);
  scag::sessions::Session * session = sessionPtr.Get();

  if (session) smsc_log_warn(logger, "SESSION IS VALID");

  scag::re::RuleEngine::Instance().process(commandDeliver1, *session);
  sm.releaseSession(sessionPtr);


}
int  main(int argc,char ** argv)
{

	smsc::logger::Logger::Init();    
	logger = smsc::logger::Logger::getInstance("scag.retst");        

	if(!logger)     
	{
	  printf("error:can`t create logger\n"); 
  	  return 0;    
	}


	init();


//	if(!initReInstance(dn))
//	{
//	  return 0;
//	}

//	dn ="./store";

//	if(!initSessionManagerInstance(dn,100))
//	{
//	  return 0;
//	}
	  
        scag::sessions::SessionPtr sess;

//	  for(;;)    
	  {
		  int scnt=1;
  
//		  sessionsOpen(scnt);
//		  sessionsDeliverSM(scnt);
//		  sessionsClose(scnt);
//		  

//        	ruleRun("deliver_sm","+79130000001",1,1,"+70001",1,1,51,0,1,sess);
fortest("+79130000001","0001");
		
//		engine->removeRule(10);
//		engine->updateRule(10);
		
//		ruleRun("deliver_sm","812345671",0,1,"897654326",0,1,51,0,1,sess);
		  
  	        //ruleRun("deliver_sm_resp","812345676",0,1,"897654321",0,1,51,0,1,sess);
		
//                ruleRun("submit_sm","812345676",0,1,"897654321",0,1,51,0,1,sess);
//		ruleRun("submit_sm_resp","812345671",0,1,"897654326",0,1,51,0,1,sess);
						
		//smsc_log_debug(logger,"\n\n");
		//printf(".");		    
		smanager->releaseSession(sess);
	  }
   
   
  return 1;
}
