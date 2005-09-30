/*
    SibInfoConsalting
    Rule Engine test application
    created by Green: green@sibinco.ru
	    
*/

#include <core/threads/Thread.hpp>
#include <scag/re/RuleEngine.h>
#include <logger/Logger.h>
#include "../RuleStatus.h"
#include "scag/transport/smpp/SmppCommand.h"

#include <fstream>
#include <jsapi.h>
#include <time.h>
#include <thread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#ifdef COMMAND_GEN
    #include "commandgen.h"
#endif

using scag::re::RuleEngine;
using scag::re::RuleStatus;
using scag::transport::smpp::SmppCommand;
using smsc::logger;


extern bool stopProcess;
//extern smsc::logger::Logger *logger;

static FILE*pFile;

RuleEngine * engine;
RuleStatus rs;
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

/*********************************************/
/* routines for RuleEngine interface working */
/*					     */
/*                                           */
/*                                           */
/*********************************************/

/**Create rules from directory */

static JSBool _createrule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 std::string dirname=JS_GetStringBytes(JS_ValueToString(cx, argv[0]));
 
 engine = new RuleEngine(dirname.c_str());
 
 if(engine)
	*rval=BOOLEAN_TO_JSVAL(true); 
 else
	*rval=BOOLEAN_TO_JSVAL(false);	   
 
 return JS_TRUE;
}

static JSBool _updaterule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
     int ruleId=JSVAL_TO_INT(argv[0]);
     updateRule(ruleId);
 
 return JS_TRUE;
}  

static JSBool _deleterule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    int ruleId=JSVAL_TO_INT(argv[0]);
     *rval=BOOLEAN_TO_JSVAL(removeRule(ruleId));
 
 return JS_TRUE;
}

/*
    RuleRun(String strTransport,String strSCAGCommandID)
    generate SCAG command by ID?
    
*/
static JSBool _rulerun(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

int ruleId=JSVAL_TO_INT(argv[0]);

    try
      {
              rs = engine->process(command);
              char buff[128];
              sprintf(buff,"%s%d","result = ",rs.result);
              smsc_log_debug(logger,buff);
      }
      catch (Exception& e)
      {
              smsc_log_error(logger,"");
              smsc_log_error(logger,std::string("Process aborted: ") + e.what());
	      
	      return JS_FALSE;
      }
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
 
 {"CreateRule",_createrule,1},
 {"UpdateRule",_updaterule,1},
 {"DeleteRule",_deleterule,1},
 {"RuleRun",_rulerun,1},
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

class CScriptRunner:public Thread
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
    engine = 0;
    
    smsc::logger::Logger::Init();
    logger = smsc::logger::Logger::getInstance("scag.ruleengine.testing");
    
    if (!logger) 
    {
	printf("error:can`t create logger\n");
	fflush(stdout);
	return 0;
    }
    engine = new RuleEngine("./rules");

    				 
    return 1;
 }
/**/

 int Execute()
 {
  if(!initre())
	return 0;
	
  runScript();
  delete engine;
  smsc::logger::Logger::Shutdown();
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
      
     //JSPipe::JSInit(cx,globalObj);

     // Execute the script
     jsval rval;
     uintN lineno = 0;

     JSString *str;
     JSBool ok = JS_EvaluateScript(cx, globalObj, script.c_str(), script.length(), scriptfilename.c_str() , lineno, &rval);
     if ( ok == JS_TRUE )
     {
      str = JS_ValueToString(cx, rval);
      char *s = JS_GetStringBytes(str);
      //std::cout << "JSExec result" << std::endl
      //          << "-------------" << std::endl
      //          << s << std::endl;
     }
     else
     {
     
      
      //smsc_log_info(logger,"Error in JavaScript file");
     
     }
    }
    else
    {
     //smsc_log_info(logger,"Unable to create the global object");
    }
    JS_DestroyContext(cx);
   }
   else
   {
    //smsc_log_info(logger,"Unable to create a context");
   }
   JS_Finish(rt);
  }
  else
  {
   //smsc_log_info(logger,"Unable to initialize the JavaScript Engine");
  }
   return 0;
 }



};