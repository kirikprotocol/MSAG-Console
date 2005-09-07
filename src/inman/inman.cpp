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
#include "inman/interaction/billing.hpp"

static const UCHAR_T VER_HIGH 	 = 0;
static const UCHAR_T VER_LOW  	 = 0;
static const UCHAR_T DEFAULT_SSN = 146;

using smsc::inman::inap::Dispatcher;
using smsc::inman::inap::Server;
using smsc::inman::inap::Factory;
using smsc::inman::inap::Session;
using smsc::inman::inap::TcapDialog;
using smsc::inman::inap::Inap;
using smsc::inman::interaction::Billing;
using smsc::inman::Console;
using smsc::logger::Logger;

using std::auto_ptr;

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

static Session* g_pSession = 0;

static void init_logger()
{
	Logger::Init();
    inapLogger = Logger::getInstance("smsc.in");
    tcapLogger = Logger::getInstance("smsc.in.tcap");
}

void init_dp(Console&, const std::vector<std::string> &args)
{
	assert( g_pSession );
	
	TcapDialog* pDialog = g_pSession->openDialog( 0 ); // 0 = new dialog id
	assert( pDialog );

	auto_ptr<Inap> inap( new Inap( pDialog ) );

	try
	{
		pDialog->beginDialog();

		auto_ptr<InitialDPSMSArg> arg( new InitialDPSMSArg() );

		inap->initialDPSMS( arg.get() );

	}
	catch(...)
	{
		g_pSession->closeDialog( pDialog );
		throw;
	}
	g_pSession->closeDialog( pDialog );

}

static void run_console()
{
  	Console console;
  	console.addItem( "init_dp", init_dp );
  	console.run("inman>");
}

int main(int argc, char** argv)
{
	if(( argc < 5 ) || ( argc > 6 ))
	{
		fprintf( stderr, "IN manager version %d.%d\n", VER_HIGH, VER_LOW );
		fprintf( stderr, "Usage: %s <scf address> <inman address> <smsc host> <smsc port> [SSN]\n", argv[0] );
		exit(1);
	}

	init_logger();

	const char* scf_addr   = argv[1];
	const char* inman_addr = argv[2];
	const char* host 	   = argv[3];
	int 		port 	   = atoi( argv[4]);
	int			SSN		   = (argc == 6) ? atoi( argv[5] ) : DEFAULT_SSN;

	smsc_log_info( inapLogger, "Starting IN manager..." );
	smsc_log_info( inapLogger, "SCF address: %s", scf_addr );
	smsc_log_info( inapLogger, "INman address: %s", inman_addr );
	smsc_log_info( inapLogger, "SMSC host: %s:%d", host, port );
	smsc_log_info( inapLogger, "SSN: %d", SSN );

	try
	{
		Factory::getInstance();

		Dispatcher*  pDisp = new Dispatcher();

		Server*	   pServer = new Server( host, port );

		smsc_log_info( inapLogger, "Server socket: 0x%X", pServer->getHandle() );

		pDisp->addListener( pServer );

		pDisp->Start();

		g_pSession = Factory::getInstance()->openSession(SSN, scf_addr, inman_addr );
		assert( g_pSession );

		run_console();

		Factory::getInstance()->closeSession( g_pSession );

		pDisp->Stop();

		
	}
	catch(const std::exception& error)
	{
		smsc_log_fatal(inapLogger, "%s", error.what() );
		fprintf( stderr, "Fatal error: %s\n", error.what() );
		exit(1);
	}

  	return(0);
}
