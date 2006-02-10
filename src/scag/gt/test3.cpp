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

#include <scag/stat/StatisticsManager.h>

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
using scag::sessions::SessionManager;
using scag::sessions::SessionManagerConfig;
using scag::sessions::CSessionKey;
using scag::sessions::SessionPtr;
using scag::transport::smpp::SmppCommand;

using namespace scag::transport::smpp;
using namespace smsc::sms;
using namespace smsc::logger;
using namespace scag::util;
using namespace scag::stat;

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

int  initReInstance( std::string& dir_name)
{

 try{

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
}

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
 cmd->set_ruleId(ruleid);

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
       sprintf(buff,"cmd:%s %s%d",cmd_name.c_str(),"result = ",rs.result);
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

int  main(int argc,char ** argv)
{


	std:string dn="./rules";
	smsc::logger::Logger::Init();    
	logger = smsc::logger::Logger::getInstance("scag.retst");        

	std::string dir="/export/home/green/install/bin/stat";
        std::string host="127.0.0.1";
	
	
	StatManConfig smcfg(dir,host,54000,54300,54400);

	StatisticsManager::init(smcfg);

	if(!logger)     
	{
	  printf("error:can`t create logger\n"); 
  	  return 0;    
	}

	if(!initReInstance(dn))
	{
	  return 0;
	}

	dn ="./store";

	if(!initSessionManagerInstance(dn,100))
	{
	  return 0;
	}

          /* Emulation for transport events */
	  
//	  for(;;)    
	  {
		  int scnt=100;
  
		  sessionsOpen(scnt);
		  sessionsDeliverSM(scnt);
		  sessionsClose(scnt);
//		  
//         	ruleRun("deliver_sm","812345671",0,1,"897654326",0,1,51,0,1,sess);
		
//		engine->removeRule(10);
//		engine->updateRule(10);
		
//		ruleRun("deliver_sm","812345671",0,1,"897654326",0,1,51,0,1,sess);
		  
  	        //ruleRun("deliver_sm_resp","812345676",0,1,"897654321",0,1,51,0,1,sess);
		
//                ruleRun("submit_sm","812345676",0,1,"897654321",0,1,51,0,1,sess);
//		ruleRun("submit_sm_resp","812345671",0,1,"897654326",0,1,51,0,1,sess);
						
		//smsc_log_debug(logger,"\n\n");
		//printf(".");		    
//		smanager->releaseSession(sess);
	  }
   
   
  return 1;
}
