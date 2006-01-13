/*
    SibInfoConsalting
    Rule Engine test application 1
    created by Green: green@sibinco.ru
*/

#include <core/threads/Thread.hpp>
#include <scag/re/RuleEngine.h>
#include "logger/Logger.h"
#include "../RuleStatus.h"
#include "scag/transport/smpp/SmppCommand.h"

#include <fstream>
#include <time.h>
#include <thread.h>
#include <stdio.h>
#include <stdlib.h>

extern bool stopProcess;
using namespace smsc::core::threads;
using namespace smsc::logger;
using namespace scag::re;

RuleEngine * RE=0;

extern Logger * logger;

class TestRunner:public Thread
{

public:
 std::string dir;

public:


/**/
 int initReInstance()
 {

    //smsc::logger::Logger::Init();
    //logger = smsc::logger::Logger::getInstance("scag.re.test");

    if (!logger)
    {
        printf("error:can`t create logger\n");
        fflush(stdout);

        return 0;
    }
       
 try{
     RuleEngine::Init(dir);
     RuleEngine& re2=RuleEngine::Instance();
         RE=&re2;
   	if(RE==0)
   	{
	  smsc_log_error(logger,"RE == 0 !");
	    return 0;
	}
    }
    catch(...){
	  smsc_log_error(logger,"can't init RE instance !");
    }

    return 1;
 }

 int Execute()
 {
  if(!initReInstance())
        return 0;

  smsc::logger::Logger::Shutdown();
  return 1;
 }

};