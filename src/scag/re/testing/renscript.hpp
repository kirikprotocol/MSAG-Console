/*
    SibInfoConsalting
    Rule Engine test application
    created by Green: green@sibinco.ru
    Javascript engine based
*/

#include <core/threads/Thread.hpp>
#include <scag/re/RuleEngine.h>
#include <logger/Logger.h>
#include <scag/re/RuleStatus.h>
#include <scag/transport/smpp/SmppCommand.h>
#include <scag/sessions/SessionManager.h>
#include <scag/sessions/Session.h>
#include <scag/bill/BillingManager.h>
#include <scag/config/ConfigManager.h>
#include <sms/sms.h>
#include <scag/transport/SCAGCommand.h>
#include <scag/exc/SCAGExceptions.h>

#include <fstream>
#include <jsapi.h>
#include <jsstr.h>
#include <time.h>
#include <thread.h>
#include <stdio.h>
#include <stdlib.h>
#include "commandgen.h"
#include <unistd.h>
#include "smpp_prop.h"

using scag::re::RuleEngine;
using scag::re::RuleStatus;
using scag::sessions::SessionManager;
using scag::sessions::SessionManagerConfig;
using scag::sessions::CSessionKey;
using scag::transport::smpp::SmppCommand;

using namespace scag::transport::smpp;
using namespace smsc::sms;
using namespace smsc::logger;
using namespace scag::util;

extern bool stopProcess;
smsc::logger::Logger *logger;

static FILE*pFile;

RuleEngine * engine=0;
SessionManager * smanager=0;
SessionManagerConfig cfg;   

SmppCommand command;
    
JSClass globalClass = 
{
 "Global", 0,
  JS_PropertyStub,  JS_PropertyStub,JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,  JS_FinalizeStub
};

static JSBool _puts(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

 if(argc!=1) 
  return JS_FALSE;
 
  std::string text=JS_GetStringBytes(JS_ValueToString(cx, argv[0]));
  
  printf(text.c_str());
  fflush(stdout);

 return JS_TRUE;

}

//------------------------
/*
JSVAL_IS_BOOLEAN 	 
JSVAL_IS_DOUBLE 	 
JSVAL_IS_GCTHING 	 
JSVAL_IS_INT 	 
JSVAL_IS_NULL 	 
JSVAL_IS_NUMBER 	 
JSVAL_IS_OBJECT 	 
JSVAL_IS_PRIMITIVE 	 
JSVAL_IS_STRING

JSTYPE_VOID, JSTYPE_OBJECT, JSTYPE_FUNCTION, 
  
 JSTYPE_STRING, JSTYPE_NUMBER, or JSTYPE_BOOLEAN.

*/
static JSBool _int2str(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

/*switch(JS_TypeOfValue(argv[0]))
{
  case JSTYPE_STRING:
	stt="string";	
    break;
  case JSTYPE_NUMBER :
    break;

  default:
			
    break;
}

   if(JSVAL_IS_STRING(argv[0]))
   {
        stt="string";
   }
	else
   {
     
   }	
*/ 
  smsc_log_debug(logger,"TEST TEST DEBUG: type is: %s",JS_GetTypeName(cx,JS_TypeOfValue(cx,argv[0])));

/*  if(argc!=2) 
    return JS_TRUE;


/* 	int count = JSVAL_TO_INT(argv[0]);
	 int value = JSVAL_TO_INT(argv[1]);

	 char fmtstr[10];

 sprintf(fmtstr,"%s%dd","%.",count);
 
	char str[17];

 sprintf(str,fmtstr,value);

	JSString jsstr;


 if(js_SetStringBytes(&jsstr, str, strlen(str))==JS_TRUE)
 {
   *rval = STRING_TO_JSVAL("jsstr;//STRING_TO_JSVAL((long)str);");

   return JS_TRUE;
 }*/

   return JS_TRUE;
}

//---------------------------

static JSBool write_file(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

 
 if(argc!=2) 
  return JS_FALSE;

 std::string filename=JS_GetStringBytes(JS_ValueToString(cx, argv[0]));
 std::string text=JS_GetStringBytes(JS_ValueToString(cx, argv[1]));

 pFile = fopen(filename.c_str(),"a+");
 
 if(pFile)
 {
	fprintf(pFile,"%s",text.c_str());

	fclose(pFile);
  
	return JS_TRUE;
 }
 else
	return JS_FALSE;

}

 

static JSBool getClock(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

 if(argc!=1)
  return JS_FALSE;

  clock_t tcl = clock();
 
 *rval = INT_TO_JSVAL(tcl);

 return JS_TRUE;

}

static JSBool _sleep(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

 if(argc!=1)
  return JS_FALSE;

 int dl=JSVAL_TO_INT(argv[0]);  // not binded
  sleep(dl);
 return JS_TRUE;

}

static JSBool _sleep_ms(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

 if(argc!=1)
  return JS_FALSE;

 timespec ts;

 ts.tv_sec=0;  // not binded
 ts.tv_nsec=JSVAL_TO_INT(argv[0]);  // not binded

 nanosleep(&ts,0);

 return JS_TRUE;

}


static JSBool _system(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

 if(argc!=1) 
  return JS_FALSE;

 std::string text=JS_GetStringBytes(JS_ValueToString(cx, argv[0]));

 system(text.c_str());

 return JS_TRUE;

}
static JSBool _exit_process(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

 
 stopProcess=true;

 return JS_TRUE;

}
static JSBool _is_stop_process(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 *rval=BOOLEAN_TO_JSVAL(stopProcess);

 return JS_TRUE;

}
static JSBool a_Yield(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 thr_yield();
 return JS_TRUE;
}

static JSBool delayMs(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

 if(argc!=1)
  return JS_FALSE;

 int dl=JSVAL_TO_INT(argv[0]);  // not binded
 clock_t tcl = clock();
 while((clock()-tcl)<dl)
  thr_yield();

 return JS_TRUE;

}
/*********************************************/
/* routines for RuleEngine interface working */
/*					                         */
/*                                           */
/*                                           */
/*********************************************/

/**Create rules from directory */

static JSBool _initReInstance(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 std::string dir_name=JS_GetStringBytes(JS_ValueToString(cx, argv[0]));

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

  if(engine)
	*rval=BOOLEAN_TO_JSVAL(true); 
  else
	*rval=BOOLEAN_TO_JSVAL(false);	   

  return JS_TRUE;
}

static JSBool _initSessionManagerInstance(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 std::string dir_name=JS_GetStringBytes(JS_ValueToString(cx, argv[0]));
 time_t tmint = (time_t)JSVAL_TO_INT(argv[1]);

try{


	 cfg.expireInterval=tmint;
         cfg.dir=dir_name;
	 SessionManager::Init(cfg);
	 SessionManager& sm2=SessionManager::Instance();
         smanager=&sm2;
	
	 if(smanager==0)
	  {
      		smsc_log_error(logger," SessionManager==0  !");

		*rval=BOOLEAN_TO_JSVAL(false); 


	  }
  
   }
   catch(...)
   {
      smsc_log_error(logger,"cant create SessionManager  !");
      *rval=BOOLEAN_TO_JSVAL(false); 

   } 
     
   *rval=BOOLEAN_TO_JSVAL(true); 
   return JS_TRUE;
}

static JSBool _initBillInstance(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
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
static JSBool _rulerun(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 CSessionKey key;

if(argc<8)
	return JS_FALSE;

  std::string cmd_name=JS_GetStringBytes(JS_ValueToString(cx, argv[0]));
  std::string str_oa  =JS_GetStringBytes(JS_ValueToString(cx, argv[1])); 

  uint8_t oa_tn =JSVAL_TO_INT(argv[2]); 
  uint8_t oa_np =JSVAL_TO_INT(argv[3]); 

  std::string str_da=JS_GetStringBytes(JS_ValueToString(cx, argv[4]));

  uint8_t da_tn =JSVAL_TO_INT(argv[5]); 
  uint8_t da_np =JSVAL_TO_INT(argv[6]); 
  uint32_t dialogid =JSVAL_TO_INT(argv[7]); 

 
  //JSVAL_TO_INT(argv[8]); 

  

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


 String2SCAGCommang(cmd,cmd_name,str_oa,oa_tn,oa_np,str_da,da_tn,da_np,dialogid);

	bool bdump=false;
	int ruleid=0;
	/***********/
	if(argc>=9)
		 bdump=(bool)(JSVAL_TO_INT(argv[8])==1);

	if(argc==10)
 		ruleid =JSVAL_TO_INT(argv[9]); 	 


/*

	if(cmd->cmdid==CommandId::DELIVERY || cmd->cmdid==CommandId::SUBMIT)
	{
		SMS sms;*/
		smsc::sms::Address oa,da ;

	 	oa.setNumberingPlan(oa_np);
	 	oa.setTypeOfNumber(oa_tn);
	 	oa.setValue(str_oa.length(),str_oa.c_str());
		
/*	 	da.setNumberingPlan(da_np);
	 	da.setTypeOfNumber(da_tn);
	 	da.setValue(str_da.length(),str_da.c_str());
	
		sms.setDestinationAddress(da);
		sms.setOriginatingAddress(oa);
		

	}
*/

 key.abonentAddr = oa;

 cmd->set_ruleId(ruleid);


 scag::sessions::Session* session =  smanager->newSession(key);;//smanager->getSession(cmd);

     if (!session) 
     {
              smsc_log_error(logger,"RuleRun():getSession('%s')==0 !",cmd_name.c_str());

		*rval=BOOLEAN_TO_JSVAL(false); 
	      return JS_TRUE;
     }

    try
      {
	      smsc_log_debug(logger,"getSession for OA='%s'ok",str_oa.c_str());

	      RuleStatus rs = engine->process(cmd,*session);

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
	      smanager->closeSession(session);		

      }
      catch (Exception& e)
      {
              smsc_log_error(logger,"cmd:%s:",cmd_name.c_str());
              smsc_log_error(logger,std::string("Process aborted: ") + e.what());
	      
		*rval=BOOLEAN_TO_JSVAL(false); 
	      return JS_TRUE;
      }
	*rval=BOOLEAN_TO_JSVAL(true); 

  return JS_TRUE;

}  
/**********************************************/  

static JSFunctionSpec Global_functions[] = {
 /*    name          native          nargs    */
 {"puts",_puts,1},
 {"DelayUs",delayMs,1},
 {"Sleep",_sleep,1},
 {"Nanosleep",_sleep_ms,1},
 {"Exit",_exit_process,1},
 {"isStop",_is_stop_process,1},
 {"yield",a_Yield,1},
 {"Clock",getClock,1},
 {"fputs",write_file,1},
 
 {"InitReInstance",_initReInstance,1},
 {"InitSessionManagerInstance",_initSessionManagerInstance,1},
 {"UpdateRule",_updaterule,1},
 {"DeleteRule",_deleterule,1},
 {"RuleRun",_rulerun,1},
 {"Int2Str",_int2str,1},
 {0}
};

static void printError(JSContext *cx, const char *message, JSErrorReport *report)
{
 int where;
 char* cstr;

 fprintf(stderr, "JSERROR: %s:%d:\n    %s\n",
  (report->filename ? report->filename : "NULL"),
  report->lineno,
  message);

 if (report->linebuf) 
 {
  fprintf(stderr, "    \"%s\"\n",report->linebuf);
  return;

 }

}

extern bool stopProcess;
using namespace smsc::core::threads;

class TestRunner:public Thread
{

public:
 std::string scriptfilename;
 
public:


/**/ 
 int initre()
 {
 
    int errorCount;
    int errorCode;
    SmppCommand command;
    smsc::logger::Logger::Init();
    logger = smsc::logger::Logger::getInstance("scag.retst");
    
    if (!logger) 
    {
	printf("error:can`t create logger\n");
	fflush(stdout);
	return 0;
    }
    //engine = new RuleEngine("./rules");
					 
    return 1;
 }
/**/

 int Execute()
 {
  
  if(!initre())
	return 0;

  	
    runScript();
   
  return 1;
 };
/**/

 int runScript()
 {
  

  std::string script;
  std::string buffer;
  std::ifstream istr(scriptfilename.c_str());

  if ( istr.is_open() )
  {
   do
   {
    std::getline(istr, buffer);
    script += buffer;
   } while (!istr.fail());

   istr.close();
  }
  else
  {
   return 1;
  }

  JSRuntime *rt = JS_Init(1000000L);

  if ( rt )
  {
   JSContext *cx = JS_NewContext(rt, 8192);
   if ( cx )
   {
    JSObject *globalObj = JS_NewObject(cx, &globalClass, 0, 0);
    if ( globalObj )
    {
     JS_InitStandardClasses(cx, globalObj);
     JS_DefineFunctions(cx, globalObj, Global_functions);
     JS_SetErrorReporter(cx, printError);

     // Init JSPipe
      
     JSSmpp_prop::JSInit(cx,globalObj);

     // Execute the script
     jsval rval;
     uintN lineno = 0;

     JSString *str;
     JSBool ok = JS_EvaluateScript(cx, globalObj, script.c_str(), script.length(), scriptfilename.c_str() , lineno, &rval);
     if ( ok == JS_TRUE )
     {
      str = JS_ValueToString(cx, rval);
      char *s = JS_GetStringBytes(str);
     }
     else
     {
     
        smsc_log_error(logger,"Error in JavaScript file");
     
     }
    }
    else
    {
     smsc_log_error(logger,"Unable to create the global object");
    }
    JS_DestroyContext(cx);
   }
   else
   {
    smsc_log_error(logger,"Unable to create a context");
   }
   JS_Finish(rt);
  }
  else
  {
   smsc_log_error(logger,"Unable to initialize the JavaScript Engine");
  }
   return 0;
 }



};