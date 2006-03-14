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
