/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <logger/Logger.h>

#include "version.inc"

#include "PersClient.h"
#include "scag/config/ConfigManager.h"

using namespace std;
using namespace smsc::logger;
using namespace scag::pers::client;
using namespace scag::pers;
using namespace scag::config;

#define ITER_CNT 10000

extern "C" static void atExitHandler(void)
{
    Logger::Shutdown();
}

int main(int argc, char* argv[])
{
    int resultCode = 0;

    Logger::Init();
    Logger* logger = Logger::getInstance("persc");

    atexit(atExitHandler);

    try{
        ConfigManager::Init();
        
        PersClient::Init("phoenix", 1222, 60, 5);
        PersClient& pc = PersClient::Instance();
        Property prop;

        time_t t = time(NULL);

        for(int i = 0; i< ITER_CNT; i++)
        {
            try{            
            char s[20];
            sprintf(s, "+7923244%04d", i);
            prop.setInt("test_val", 234567, FIXED, -1, 20);
            pc.SetProperty(PT_ABONENT, s, prop);

            prop.setBool("test_val_bool", false, R_ACCESS, -1, 25);
            pc.SetProperty(PT_ABONENT, s, prop);

            prop.setString("test_val_string", "test_string", W_ACCESS, -1, 25);
            pc.SetProperty(PT_ABONENT, s, prop);

            prop.setDate("test_val_date", 111111, INFINIT, -1, 25);
            pc.SetProperty(PT_ABONENT, s, prop);

            pc.GetProperty(PT_ABONENT, s, "test_val", prop);
            smsc_log_debug(logger,  ">>ABONENT %s: get int %s", s, prop.toString().c_str());

//            sleep(15);
            pc.GetProperty(PT_ABONENT, s, "test_val_bool", prop);
            smsc_log_debug(logger,  ">>ABONENT %s: get bool %s", s, prop.toString().c_str());

            pc.GetProperty(PT_ABONENT, s, "test_val_string", prop);
            smsc_log_debug(logger,  ">>ABONENT: get string %s", prop.toString().c_str());

            pc.GetProperty(PT_ABONENT, s, "test_val_date", prop);
            smsc_log_debug(logger,  ">>ABONENT: get string1 %s", prop.toString().c_str());

            prop.setInt("test_val", -123, FIXED, -1, 20);
            pc.IncProperty(PT_ABONENT, s, prop);
            pc.GetProperty(PT_ABONENT, s, "test_val", prop);
            smsc_log_debug(logger,  ">>ABONENT: get int(after inc) %s", prop.toString().c_str());

            pc.DelProperty(PT_ABONENT, s, "test_val");
            smsc_log_debug(logger,  ">>ABONENT: del int");

//            sleep(15);
            prop.setInt("test_val", 234567, FIXED, -1, 20);
            pc.SetProperty(PT_SERVICE, i + 1, prop);

            prop.setBool("test_val_bool", false, R_ACCESS, -1, 25);
            pc.SetProperty(PT_PROVIDER, i + 1, prop);

            prop.setString("test_val_string", "test_string", W_ACCESS, -1, 25);
            pc.SetProperty(PT_OPERATOR, i + 1, prop);

            prop.setDate("test_val_date", 111111, INFINIT, -1, 25);
            pc.SetProperty(PT_OPERATOR, i + 1, prop);

            pc.GetProperty(PT_SERVICE, i + 1, "test_val", prop);
            smsc_log_debug(logger,  ">>SERVICE: get int %s", prop.toString().c_str());

            pc.GetProperty(PT_PROVIDER, i + 1, "test_val_bool", prop);
            smsc_log_debug(logger,  ">>OPERATOR: get bool %s", prop.toString().c_str());

            pc.GetProperty(PT_OPERATOR, i + 1, "test_val_string", prop);
            smsc_log_debug(logger,  ">>OPERATOR: get string %s", prop.toString().c_str());

            pc.GetProperty(PT_OPERATOR, i + 1, "test_val_date", prop);
            smsc_log_debug(logger,  ">>OPERATOR: get date %s", prop.toString().c_str());

            prop.setInt("test_val", -123, FIXED, -1, 20);
            pc.IncProperty(PT_SERVICE, i + 1, prop);
            pc.GetProperty(PT_SERVICE, i + 1, "test_val", prop);
            smsc_log_debug(logger,  ">>SERVICE: get int(after inc) %s", prop.toString().c_str());

            prop.setInt("test_val", -123, FIXED, -1, 20);
            pc.IncModProperty(PT_SERVICE, i + 1, prop, 10);
            pc.GetProperty(PT_SERVICE, i + 1, "test_val", prop);
            smsc_log_debug(logger,  ">>SERVICE: get int(after inc mod) %s", prop.toString().c_str());

            pc.DelProperty(PT_SERVICE, i + 1, "test_val");
            smsc_log_debug(logger,  ">>ABONENT: del int(int)");
            }
            catch(PersClientException &e)
            {
                smsc_log_error(logger, "persclientexception: %s", e.what());
            }

            try{
            pc.DelProperty(PT_SERVICE, i + 1, "test_val");
            smsc_log_debug(logger,  ">>ABONENT: del int(int)");
            }
            catch(PersClientException &e)
            {
                smsc_log_debug(logger, "property already deleted...");
            }

            smsc_log_debug(logger,  "end");
        }
        t = time(NULL) - t;
        if(t)
            smsc_log_error(logger,  "timings: %d sec, %d req/s", t, 24*ITER_CNT/t);
    }
    catch (PersClientException& exc) 
    {
        smsc_log_error(logger, "PersClientException: %s Exiting.", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc) 
    {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) 
    {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    }
    catch (...) 
    {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -5;
    }

    return resultCode;
}
