#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#include "logger/Logger.h"
//#include "core/network/Socket.hpp"
#include "inman/common/console.hpp"
#include "inman/interaction/serializer.hpp"
#include "inman/interaction/messages.hpp"
#include "core/threads/Thread.hpp" 	

using smsc::core::threads::Thread;
using smsc::core::network::Socket;
using smsc::logger::Logger;
using smsc::inman::common::Console;
using smsc::inman::interaction;
using smsc::inman::interaction::ObjectPipe;
using smsc::inman::interaction::SerializerInap;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::SmscHandler;
using smsc::inman::interaction::SmscCommand;
using smsc::inman::interaction::DeliverySmsResult_t;


class Facade;

static Facade* g_pFacade = 0;
static int dialogId = 1;

class Facade : public Thread, public SmscHandler
{
		Socket*				socket;
		ObjectPipe* 		pipe;
		Logger*  			logger;
		DeliverySmsResult_t delivery;
	public:

		Facade(Socket* sock) 
			: socket( sock )
			, pipe( new ObjectPipe( sock, SerializerInap::getInstance() ) )
			, logger( Logger::getInstance("smsc.inman.Facade") )
			, delivery( smsc::inman::interaction::DELIVERY_SUCCESSED )
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

		void setDeliveryResult( DeliverySmsResult_t dr )
		{
			delivery = dr;
		}

		void onChargeSmsResult(ChargeSmsResult* result)
		{
			assert( result );
			if( result->GetValue() == smsc::inman::interaction::CHARGING_POSSIBLE )
			{
				fprintf( stdout, "ChargeSmsResult( CHARGING_POSSIBLE ) received\n");
				DeliverySmsResult op( delivery );
				op.setObjectId( result->getObjectId() );
				sendDeliverySmsResult( &op );
			}
			else
			{
				fprintf( stdout, "ChargeSmsResult( CHARGING_NOT_POSSIBLE ) received\n");
			}
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


void send()
{
	ChargeSms op;

	op.setObjectId( dialogId++ );

	op.setDestinationSubscriberNumber( ".1.1.79139859489" ); // missing for MT
	op.setCallingPartyNumber( ".1.1.79139343290" );
	op.setIMSI( "250013900405871" );
	op.setLocationInformationMSC( ".1.1.79139860001" );
	op.setSMSCAddress(".1.1.79029869990");

	time_t tm;
	time( &tm );

	op.setTimeAndTimezone( tm );
	op.setTPShortMessageSpecificInfo( 0x11 );
	op.setTPValidityPeriod( 60*5 );
	op.setTPProtocolIdentifier( 0x00 );
	op.setTPDataCodingScheme( 0x08 );

	g_pFacade->sendChargeSms( &op );
}

void cmd_charge(Console&, const std::vector<std::string> &args)
{
	g_pFacade->setDeliveryResult( smsc::inman::interaction::DELIVERY_SUCCESSED );
	send();
}

void cmd_check(Console&, const std::vector<std::string> &args)
{
	g_pFacade->setDeliveryResult( smsc::inman::interaction::DELIVERY_FAILED );
	send();
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
  		console.addItem( "check",  cmd_check );

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