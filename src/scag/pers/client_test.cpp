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

#define ITER_CNT 100

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
        
        smsc_log_debug(logger, "Host: %s:%d. Requests: %d", argc > 1 ? argv[1] : "localhost", argc > 2 ? atoi(argv[2]) : 22890, 45 * ITER_CNT);        
        PersClient::Init(argc > 1 ? argv[1] : "localhost", argc > 2 ? atoi(argv[2]) : 22890, 60, 5);

        PersClient& pc = PersClient::Instance();
        Property prop;
        SerialBuffer sb;

        time_t t = time(NULL);

        for(int i = 0; i< ITER_CNT; i++)
        {
            char s[20], tv[20], tvs[30], tvb[30], tvd[30];		
            try{ 

            sprintf(s, "892324%05d", i);
            sprintf(tv, "test_val%05d", i);			
            sprintf(tvs, "test_val_string%05d", i);						
            sprintf(tvb, "test_val_bool%05d", i);						
            sprintf(tvd, "test_val_date%05d", i);						
            prop.setInt(tv, 234567, FIXED, -1, 20);
            pc.SetProperty(PT_ABONENT, s, prop);

            prop.setBool(tvb, false, R_ACCESS, -1, 25);
            pc.SetProperty(PT_ABONENT, s, prop);

            prop.setString(tvs, "test_string", W_ACCESS, -1, 25);
            pc.SetProperty(PT_ABONENT, s, prop);

            prop.setDate(tvd, 111111, INFINIT, -1, 25);
            pc.SetProperty(PT_ABONENT, s, prop);

            pc.GetProperty(PT_ABONENT, s, tv, prop);
            smsc_log_debug(logger,  ">>ABONENT %s: get int %s", s, prop.toString().c_str());

            pc.GetProperty(PT_ABONENT, s, tvb, prop);
            smsc_log_debug(logger,  ">>ABONENT %s: get bool %s", s, prop.toString().c_str());

            pc.GetProperty(PT_ABONENT, s, tvs, prop);
            smsc_log_debug(logger,  ">>ABONENT: get string %s", prop.toString().c_str());

            pc.GetProperty(PT_ABONENT, s, tvd, prop);
            smsc_log_debug(logger,  ">>ABONENT: get string1 %s", prop.toString().c_str());

            prop.setInt(tv, -123, FIXED, -1, 20);
            pc.IncProperty(PT_ABONENT, s, prop);
            pc.GetProperty(PT_ABONENT, s, tv, prop);
            smsc_log_debug(logger,  ">>ABONENT: get int(after inc) %s", prop.toString().c_str());
			
            pc.DelProperty(PT_ABONENT, s, tv);
            smsc_log_debug(logger,  ">>ABONENT: del int");

            prop.setInt(tv, 234567, FIXED, -1, 20);
            pc.SetProperty(PT_SERVICE, i + 1, prop);

            prop.setBool(tvb, false, R_ACCESS, -1, 25);
            pc.SetProperty(PT_PROVIDER, i + 1, prop);

            prop.setString(tvs, "test_string", W_ACCESS, -1, 25);
            pc.SetProperty(PT_OPERATOR, i + 1, prop);

            prop.setDate(tvd, 111111, INFINIT, -1, 25);
            pc.SetProperty(PT_OPERATOR, i + 1, prop);

            pc.GetProperty(PT_SERVICE, i + 1, tv, prop);
            smsc_log_debug(logger,  ">>SERVICE: get int %s", prop.toString().c_str());

            pc.GetProperty(PT_PROVIDER, i + 1, tvb, prop);
            smsc_log_debug(logger,  ">>OPERATOR: get bool %s", prop.toString().c_str());

            pc.GetProperty(PT_OPERATOR, i + 1, tvs, prop);
            smsc_log_debug(logger,  ">>OPERATOR: get string %s", prop.toString().c_str());

            pc.GetProperty(PT_OPERATOR, i + 1, tvd, prop);
            smsc_log_debug(logger,  ">>OPERATOR: get date %s", prop.toString().c_str());

            prop.setInt(tv, -123, FIXED, -1, 20);
            pc.IncProperty(PT_SERVICE, i + 1, prop);
            pc.GetProperty(PT_SERVICE, i + 1, tv, prop);
            smsc_log_debug(logger,  ">>SERVICE: get int(after inc) %s", prop.toString().c_str());

            prop.setInt(tv, -123, FIXED, -1, 20);
            pc.IncModProperty(PT_SERVICE, i + 1, prop, 10);
            pc.GetProperty(PT_SERVICE, i + 1, tv, prop);
            smsc_log_debug(logger,  ">>SERVICE: get int(after inc mod) %s", prop.toString().c_str());

//batch
            pc.PrepareBatch(sb);

            prop.setInt(tv, 234567, FIXED, -1, 20);
            pc.SetPropertyPrepare(PT_ABONENT, s, prop, sb);

            prop.setBool(tvb, false, R_ACCESS, -1, 25);
            pc.SetPropertyPrepare(PT_ABONENT, s, prop, sb);

            prop.setString(tvs, "test_string", W_ACCESS, -1, 25);
            pc.SetPropertyPrepare(PT_ABONENT, s, prop, sb);

            prop.setDate(tvd, 111111, INFINIT, -1, 25);
            pc.SetPropertyPrepare(PT_ABONENT, s, prop, sb);

            pc.GetPropertyPrepare(PT_ABONENT, s, tv, sb);
            pc.GetPropertyPrepare(PT_ABONENT, s, tvb, sb);
            pc.GetPropertyPrepare(PT_ABONENT, s, tvs, sb);
            pc.GetPropertyPrepare(PT_ABONENT, s, tvd, sb);

            prop.setInt(tv, -123, FIXED, -1, 20);
            pc.IncPropertyPrepare(PT_ABONENT, s, prop, sb);
            pc.GetPropertyPrepare(PT_ABONENT, s, tv, sb);
			
            pc.DelPropertyPrepare(PT_ABONENT, s, tv, sb);

            prop.setInt(tv, 234567, FIXED, -1, 20);
            pc.SetPropertyPrepare(PT_SERVICE, i + 1, prop, sb);

            prop.setBool(tvb, false, R_ACCESS, -1, 25);
            pc.SetPropertyPrepare(PT_PROVIDER, i + 1, prop, sb);

            prop.setString(tvs, "test_string", W_ACCESS, -1, 25);
            pc.SetPropertyPrepare(PT_OPERATOR, i + 1, prop, sb);

            prop.setDate(tvd, 111111, INFINIT, -1, 25);
            pc.SetPropertyPrepare(PT_OPERATOR, i + 1, prop, sb);

            pc.GetPropertyPrepare(PT_SERVICE, i + 1, tv, sb);

            pc.GetPropertyPrepare(PT_PROVIDER, i + 1, tvb, sb);

            pc.GetPropertyPrepare(PT_OPERATOR, i + 1, tvs, sb);

            pc.GetPropertyPrepare(PT_OPERATOR, i + 1, tvd, sb);

            prop.setInt(tv, -123, FIXED, -1, 20);
            pc.IncModPropertyPrepare(PT_SERVICE, i + 1, prop, 10, sb);
            pc.GetPropertyPrepare(PT_SERVICE, i + 1, tv, sb);

            pc.DelPropertyPrepare(PT_SERVICE, i + 1, tv, sb);

            pc.FinishPrepareBatch(22, sb);
            pc.RunBatch(sb);

            pc.SetPropertyResult(sb);
            pc.SetPropertyResult(sb);
            pc.SetPropertyResult(sb);
            pc.SetPropertyResult(sb);

            pc.GetPropertyResult(prop, sb);
            smsc_log_debug(logger,  "BATCH >>ABONENT %s: get int %s", s, prop.toString().c_str());

            pc.GetPropertyResult(prop, sb);
            smsc_log_debug(logger,  "BATCH >>ABONENT %s: get bool %s", s, prop.toString().c_str());

            pc.GetPropertyResult(prop, sb);
            smsc_log_debug(logger,  "BATCH >>ABONENT: get string %s", prop.toString().c_str());

            pc.GetPropertyResult(prop, sb);
            smsc_log_debug(logger,  "BATCH >>ABONENT: get string1 %s", prop.toString().c_str());

            pc.IncPropertyResult(sb);
            pc.GetPropertyResult(prop, sb);
            smsc_log_debug(logger,  "BATCH >>ABONENT: get int(after inc) %s", prop.toString().c_str());
			
            pc.DelPropertyResult(sb);
            smsc_log_debug(logger,  "BATCH >>ABONENT: del int");

            pc.SetPropertyResult(sb);
            pc.SetPropertyResult(sb);
            pc.SetPropertyResult(sb);
            pc.SetPropertyResult(sb);

            pc.GetPropertyResult(prop, sb);
            smsc_log_debug(logger,  "BATCH >>SERVICE: get int %s", prop.toString().c_str());

            pc.GetPropertyResult(prop, sb);
            smsc_log_debug(logger,  "BATCH >>OPERATOR: get bool %s", prop.toString().c_str());

            pc.GetPropertyResult(prop, sb);
            smsc_log_debug(logger,  "BATCH >>OPERATOR: get string %s", prop.toString().c_str());

            pc.GetPropertyResult(prop, sb);
            smsc_log_debug(logger,  "BATCH >>OPERATOR: get date %s", prop.toString().c_str());

            int kkk = pc.IncModPropertyResult(sb);
            pc.GetPropertyResult(prop, sb);
            smsc_log_debug(logger,  "BATCH >>SERVICE: get int(after inc mod)(%d) %s", kkk, prop.toString().c_str());

            pc.DelPropertyResult(sb);
            smsc_log_debug(logger,  "BATCH >>ABONENT: del int(int)");
            
            }
            catch(PersClientException &e)
            {
                smsc_log_error(logger, "persclientexception: %s", e.what());
            }

            if(pc.DelProperty(PT_SERVICE, i + 1, tv))
	            smsc_log_debug(logger,  ">>ABONENT: del int(int)");
			else
                smsc_log_debug(logger, "property already deleted...");

            smsc_log_debug(logger,  "end");
        }
        t = time(NULL) - t;
        if(t)
            smsc_log_error(logger,  "timings: %d sec, %d req/s", t, 45 * ITER_CNT/t); //24
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
