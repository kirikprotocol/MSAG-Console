/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <logger/Logger.h>

#include "version.inc"

#include "PersClient.h"

using namespace std;
using namespace smsc::logger;
using namespace scag::pers::client;
using namespace scag::pers;

static Logger *logger;

extern "C" static void atExitHandler(void)
{
    Logger::Shutdown();
}

int main(int argc, char* argv[])
{
	int resultCode = 0;

	Logger::Init();
	logger = Logger::getInstance("persc");

	atexit(atExitHandler);

	try{
		PersClient::Init("localhost", 1200, 60);
		PersClient& pc = PersClient::Instance();
		Property prop;

		for(int i = 0; i< 1000; i++)
		{
		prop.setInt("test_val", 234567, FIXED, -1, 20);
		pc.SetProperty(PT_ABONENT, "+79232446251", prop);

		prop.setBool("test_val_bool", false, R_ACCESS, -1, 25);
		pc.SetProperty(PT_ABONENT, "+79232446251", prop);

		prop.setString("test_val_string", L"test_string", W_ACCESS, -1, 25);
		pc.SetProperty(PT_ABONENT, "+79232446251", prop);

		prop.setDate("test_val_date", 111111, INFINIT, -1, 25);
		pc.SetProperty(PT_ABONENT, "+79232446251", prop);

		pc.GetProperty(PT_ABONENT, "+79232446251", "test_val", prop);
		smsc_log_debug(logger,  ">>ABONENT: get int %s", prop.toString().c_str());

		pc.GetProperty(PT_ABONENT, "+79232446251", "test_val_bool", prop);
		smsc_log_debug(logger,  ">>ABONENT: get bool %s", prop.toString().c_str());

		pc.GetProperty(PT_ABONENT, "+79232446251", "test_val_string", prop);
		smsc_log_debug(logger,  ">>ABONENT: get string %s", prop.toString().c_str());

		pc.GetProperty(PT_ABONENT, "+79232446251", "test_val_date", prop);
		smsc_log_debug(logger,  ">>ABONENT: get string1 %s", prop.toString().c_str());

		pc.IncProperty(PT_ABONENT, "+79232446251", "test_val", -123);
		pc.GetProperty(PT_ABONENT, "+79232446251", "test_val", prop);
		smsc_log_debug(logger,  ">>ABONENT: get int(after inc) %s", prop.toString().c_str());

		pc.DelProperty(PT_ABONENT, "+79232446251", "test_val");
		smsc_log_debug(logger,  ">>ABONENT: del int");


		prop.setInt("test_val", 234567, FIXED, -1, 20);
		pc.SetProperty(PT_SERVICE, 12, prop);

		prop.setBool("test_val_bool", false, R_ACCESS, -1, 25);
		pc.SetProperty(PT_PROVIDER, 12, prop);

		prop.setString("test_val_string", L"test_string", W_ACCESS, -1, 25);
		pc.SetProperty(PT_OPERATOR, 12, prop);

		prop.setDate("test_val_date", 111111, INFINIT, -1, 25);
		pc.SetProperty(PT_OPERATOR, 12, prop);

		pc.GetProperty(PT_SERVICE, 12, "test_val", prop);
		smsc_log_debug(logger,  ">>SERVICE: get int %s", prop.toString().c_str());

		pc.GetProperty(PT_PROVIDER, 12, "test_val_bool", prop);
		smsc_log_debug(logger,  ">>OPERATOR: get bool %s", prop.toString().c_str());

		pc.GetProperty(PT_OPERATOR, 12, "test_val_string", prop);
		smsc_log_debug(logger,  ">>OPERATOR: get string %s", prop.toString().c_str());

		pc.GetProperty(PT_OPERATOR, 12, "test_val_date", prop);
		smsc_log_debug(logger,  ">>OPERATOR: get date %s", prop.toString().c_str());

		pc.IncProperty(PT_SERVICE, 12, "test_val", -123);
		pc.GetProperty(PT_SERVICE, 12, "test_val", prop);
		smsc_log_debug(logger,  ">>SERVICE: get int(after inc) %s", prop.toString().c_str());

		pc.DelProperty(PT_SERVICE, 12, "test_val");
		smsc_log_debug(logger,  ">>ABONENT: del int(int)");

		smsc_log_debug(logger,  "end");
		}
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
