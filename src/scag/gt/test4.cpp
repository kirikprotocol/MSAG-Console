/*
    SibInfoConsalting
    Rule Engine teest 3
    created by Green: green@sibinco.ru
    Javascript engine based
*/
#define TESTING

#define TESTING_ADMIN

#include <logger/Logger.h>
#include "scag/admin/SCAGCommandDispatcher.h"
#include "scag/admin/SCAGSocketListener.h"

#include <stddef.h>
#include <fstream>
#include <time.h>
#include <thread.h>
#include <stdio.h>
#include <stdlib.h>
#include "commandgen.h"
#include <unistd.h>
smsc::logger::Logger *logger;
    
void testAdmin(std::string admin_host,int admin_port)
{
	
	 scag::admin::SCAGCommandDispatcher::startGw();
     smsc_log_debug(logger,"SMPP GW started\n");


      scag::admin::SCAGSocketListener listener;
      listener.init(admin_host.c_str(), admin_port);

      //scag::registerScagSignalHandlers(&listener);

      listener.Start();

      smsc_log_debug(logger,"SMPP GW admin listener started\n");

      //running...
      smsc_log_debug(logger,"running...\n");

      listener.WaitFor();

      //smsc_log_debug(logger,"SCAG shutdown...\n");
     scag::admin::SCAGCommandDispatcher::stopGw();
     smsc_log_debug(logger,"SCAG stopped\n");

      //smsc::util::config::Manager::deinit();

      //smsc_log_debug(logger,"all finished\n");

}
void main()
{
	smsc::logger::Logger::Init();    
	logger = smsc::logger::Logger::getInstance("scag.retst");        

testAdmin("localhost",29992);
}