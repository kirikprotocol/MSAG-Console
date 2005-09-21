#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#include "logger/Logger.h"
#include "core/network/Socket.hpp"
#include "inman/common/console.hpp"
#include "inman/interaction/serializer.hpp"
#include "inman/interaction/messages.hpp"
#include "core/threads/Thread.hpp" 	

using smsc::core::threads::Thread;
using smsc::core::network::Socket;
using smsc::logger::Logger;
using smsc::inman::common::Console;
using smsc::inman::interaction::ObjectPipe;
using smsc::inman::interaction::Serializer;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::SmscHandler;
using smsc::inman::interaction::SmscCommand;


class Facade;

static Facade* g_pFacade = 0;


class Facade : public Thread, public SmscHandler
{
		Socket*		socket;
		ObjectPipe* pipe;
		Logger*  	logger;
	public:

		Facade(Socket* sock) 
			: socket( sock )
			, pipe( new ObjectPipe( sock ) )
			, logger( Logger::getInstance("smsc.inman.Facade") )
		{
		}

		virtual ~Facade()
		{
			delete pipe;
			delete socket;
		}

		void sendChargeSms(ChargeSms* op)
		{              	
			pipe->send(op);
		}

		void sendDeliverySmsResult(DeliverySmsResult* op )
		{
			pipe->send(op);
		}

		void onChargeSmsResult(ChargeSmsResult* sms)
		{
			fprintf( stdout, "ChargeSmsResult received\n");
		}

		virtual int  Execute()
		{
			for(;;)
			{
				fd_set read;
				FD_ZERO( &read );
				FD_SET( socket->getSocket(), &read );
  				int n = select(  socket->getSocket()+1, &read, 0, 0, 0 );
  				if( n > 0 )
  				{
  					SmscCommand* cmd = static_cast<SmscCommand*>(pipe->receive());
  					assert( cmd );
  					cmd->handle( this );
  				}
			}
			return 0;
		}
	
};


void cmd_charge(Console&, const std::vector<std::string> &args)
{
	ChargeSms op;
	g_pFacade->sendChargeSms( &op );
	
}

void cmd_delivery(Console&, const std::vector<std::string> &args)
{
	DeliverySmsResult op;
	g_pFacade->sendDeliverySmsResult( &op );
}

int main(int argc, char** argv)
{
	Logger::Init();
	
	if( argc != 3 )
	{
		fprintf( stderr, "Usage: %s <host> <port>\n", argv[0] );
		exit(1);
	}

	const char* host = argv[1];
	int port = atoi( argv[2]);

	fprintf( stdout, "Connecting to IN manager at %s:%d...\n", host, port );
	
	try
	{
		Socket* sock = new Socket();

		if( sock->Init( host, port, 1000 ) != 0 )
		{
			fprintf( stderr, "Can't init socket: %s (%d)\n", strerror( errno ), errno );
			throw std::runtime_error("Can't init socket");
		}

		if( sock->Connect() != 0 )
		{
			fprintf( stderr, "Can't connect socket: %s (%d)\n", strerror( errno ), errno );
			throw std::runtime_error("Can't connect socket");
		}
		
		g_pFacade = new Facade( sock );

 		Console console;
  		console.addItem( "charge", cmd_charge );
  		console.addItem( "delivery", cmd_delivery );

		g_pFacade->Start();

  		console.run("inman>");

	}
	catch(const std::exception& error)
	{
		fprintf(stderr, "Fatal error: %s\n", error.what() );
	}

	delete g_pFacade;

	exit(0);
}