static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>
#include <memory>

#include "console.hpp"
#include "logger/Logger.h"
#include "inman/inap/dispatcher.hpp"
#include "inman/inap/server.hpp"
#include "inman/inap/factory.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/inap/inap.hpp"
#include "billing.hpp"

static const UCHAR_T VER_HIGH 	 = 0;
static const UCHAR_T VER_LOW  	 = 0;
static const UCHAR_T DEFAULT_SSN = 146;

using smsc::inman::inap::Dispatcher;
using smsc::inman::inap::Server;
using smsc::inman::inap::Factory;
using smsc::inman::inap::Session;
using smsc::inman::inap::TcapDialog;
using smsc::inman::inap::Inap;
using smsc::inman::Billing;
using smsc::inman::Console;
using smsc::logger::Logger;

using std::auto_ptr;
using std::runtime_error;

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

static Billing* g_pBilling   = 0;

static void init_logger()
{
	Logger::Init();
    inapLogger = Logger::getInstance("smsc.in");
    tcapLogger = Logger::getInstance("smsc.in.tcap");
}

void initdp(Console&, const std::vector<std::string> &args)
{
	g_pBilling->initialDPSMS();
	fprintf( stdout, "OK.\n" );
}

static void run_console()
{
  	Console console;
  	console.addItem( "initdp", initdp );
  	console.run("inman>");
}

int main(int argc, char** argv)
{
	if(( argc < 5 ) || ( argc > 6 ))
	{
		fprintf( stderr, "IN manager version %d.%d\n", VER_HIGH, VER_LOW );
		fprintf( stderr, "Usage: %s <inman/ssf address> <scf address> <smsc host> <smsc port> [SSN]\n", argv[0] );
		exit(1);
	}

	init_logger();

	const char* ssf_addr   = argv[1];
	const char* scf_addr   = argv[2];
	const char* host 	   = argv[3];
	int 		port 	   = atoi( argv[4]);
	int			SSN		   = (argc == 6) ? atoi( argv[5] ) : DEFAULT_SSN;

	smsc_log_info( inapLogger, "Starting IN manager..." );
	smsc_log_info( inapLogger, "SSF address: %s", ssf_addr );
	smsc_log_info( inapLogger, "SCF address: %s", scf_addr );
	smsc_log_info( inapLogger, "SMSC host: %s:%d", host, port );
	smsc_log_info( inapLogger, "SSN: %d", SSN );

	try
	{
		Factory::getInstance();

		Dispatcher disp;
		Server	   server(host, port);

		disp.Start();
		server.Start();

		Session* pSession = Factory::getInstance()->openSession(SSN, ssf_addr, scf_addr );
		assert( pSession );

		g_pBilling = new Billing( pSession );
		assert( g_pBilling );

		run_console();

		delete g_pBilling;

		Factory::getInstance()->closeSession( pSession );

		server.Stop();
		disp.Stop();
	}
	catch(const std::exception& error)
	{
		smsc_log_fatal(inapLogger, "%s", error.what() );
		fprintf( stderr, "Fatal error: %s\n", error.what() );
		exit(1);
	}

  	return(0);
}
