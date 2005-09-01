static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>

#include "console.hpp"
#include "logger/Logger.h"
#include "inman/inap/factory.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/inap/inap.hpp"
#include "inman/interaction/billing.hpp"

static const UCHAR_T VER_HIGH 	 = 0;
static const UCHAR_T VER_LOW  	 = 0;
static const UCHAR_T DEFAULT_SSN = 146;

using smsc::inman::inap::Factory;
using smsc::inman::inap::Session;
using smsc::inman::inap::TcapDialog;
using smsc::inman::inap::Inap;
using smsc::inman::interaction::Billing;
using smsc::inman::Console;
using smsc::logger::Logger;

namespace smsc
{
	namespace inman
	{
		namespace inap
		{
    		Logger* inapLogger;
    		Logger* tcapLogger;
    	}
	}
};

using smsc::inman::inap::inapLogger;
using smsc::inman::inap::tcapLogger;

static void init_logger()
{
	Logger::Init();
    inapLogger = Logger::getInstance("smsc.in");
    tcapLogger = Logger::getInstance("smsc.in.tcap");
}

static void run_console()
{
  	Console console;
  	console.run("inman>");
}

int main(int argc, char** argv)
{
	if(( argc < 5 ) || ( argc > 6 ))
	{
		fprintf( stderr, "IN manager version %d.%d\n", VER_HIGH, VER_LOW );
		fprintf( stderr, "Usage: %s <smc address> <inman address> <smsc host> <smsc port> [SSN]\n", argv[0] );
		exit(1);
	}

	init_logger();

	const char* smc_addr   = argv[1];
	const char* inman_addr = argv[2];
	const char* host 	   = argv[3];
	int 		port 	   = atoi( argv[4]);
	int			SSN		   = (argc == 6) ? atoi( argv[5] ) : DEFAULT_SSN;

	smsc_log_info( inapLogger, "Starting IN manager...\n" );
	smsc_log_info( inapLogger, "SMC   address: %s", smc_addr );
	smsc_log_info( inapLogger, "INman address: %s", inman_addr );
	smsc_log_info( inapLogger, "SMSC  host   : %s:%d", host, port );
	smsc_log_info( inapLogger, "SSN		     : %d", SSN );

	try
	{
		Session* pSession = Factory::getInstance()->openSession( SSN, smc_addr, inman_addr, host, port );
		run_console();
		Factory::getInstance()->closeSession( pSession );
	}
	catch(const std::exception& error)
	{
		smsc_log_fatal(inapLogger, "%s", error.what() );
		fprintf( stderr, "Fatal error: %s\n", error.what() );
		exit(1);
	}

  	return(0);
}
