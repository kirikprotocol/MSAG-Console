/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include "version.inc"

#include "PersClient.h"

using namespace std;
using namespace smsc::logger;
using namespace scag::pers::client;

static Logger *logger;

extern "C" static void atExitHandler(void)
{
    Logger::Shutdown();
}

int main(int argc, char* argv[])
{
	int resultCode = 0;
	string host;
	int port = 9988;
	int maxClientCount = 100;

	Logger::Init();
	logger = Logger::getInstance("persc");

	atexit(atExitHandler);

	try{
		PersClient pc;
		pc.init("localhost", 9999, 60);
/*		Property prop;
		prop.setInt("test_val", 234567, FIXED, -1, 20);
		smsc_log_debug(logger,  "setProperty: %s", prop.toString().c_str());
		StringProfileKey spk("+79232446251");
		AbonentStore.setProperty(spk, &prop);
		prop.setBool("test_val_bool", false, R_ACCESS, -1, 25);
		AbonentStore.setProperty(spk, &prop);
		prop.setString("test_val_string", L"test_string", W_ACCESS, -1, 25);
		AbonentStore.setProperty(spk, &prop);
		prop.setDate("test_val_string1", 111111, INFINIT, -1, 25);
		AbonentStore.setProperty(spk, &prop);
		auto_ptr<Property> pp( AbonentStore.getProperty(spk, "test_val"));
		if(pp.get() != NULL)
			smsc_log_debug(logger,  "pers %s", pp.get()->toString().c_str());
		smsc_log_debug(logger,  "end");

		prop.setInt("test_val", 234567, FIXED, -1, 20);
		smsc_log_debug(logger,  "setProperty: %s", prop.toString().c_str());
		IntProfileKey ipk(12);
		ServiceStore.setProperty(ipk, &prop);
		prop.setBool("test_val_bool", false, R_ACCESS, -1, 25);
		ServiceStore.setProperty(ipk, &prop);
		prop.setString("test_val_string", L"test_string", W_ACCESS, -1, 25);
		ServiceStore.setProperty(ipk, &prop);
		prop.setDate("test_val_string1", 111111, INFINIT, -1, 25);
		ServiceStore.setProperty(ipk, &prop);
		auto_ptr<Property> pp1( ServiceStore.getProperty(ipk, "test_val"));
		if(pp1.get() != NULL)
			smsc_log_debug(logger,  ">>pers int %s", pp1.get()->toString().c_str());
		auto_ptr<Property> pp2( ServiceStore.getProperty(ipk, "test_val_string"));
		if(pp2.get() != NULL && pp2.get()->getStringValue() == L"test_string")
			smsc_log_debug(logger,  "####>>pers int %s", pp1.get()->toString().c_str());
		smsc_log_debug(logger,  "end");*/

    }
    catch (PersClientException& exc) 
    {
        smsc_log_error(logger, "PersClientException: %d Exiting.", exc.getType());
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
