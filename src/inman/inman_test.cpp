#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

//#include "console.hpp"
#include "core/network/Socket.hpp"
#include "inman/interaction/serializer.hpp"
#include "inman/interaction/messages.hpp"

using smsc::core::network::Socket;
//using smsc::inman::Console;
using smsc::inman::interaction::Serializer;
//using smsc::inman::interaction::StartMessage;

static const int SOCKET_TIMEOUT = 1000;
static Socket 	 g_socket;

void start()
{
/*	StartMessage startMessage;
	ObjectBuffer buffer(16);
	Serializer::getInstance()->serialize( &startMessage, buffer );
	g_socket.Write( buffer.get(), buffer.GetPos() );
	*/
}


int main(int argc, char** argv)
{
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
		if( g_socket.Init( host, port, SOCKET_TIMEOUT ) != 0 )
		{
			fprintf( stderr, "Can't init socket: %s (%d)\n", strerror( errno ), errno );
			throw std::runtime_error("Can't init socket");
		}

		if( g_socket.Connect() != 0 )
		{
			fprintf( stderr, "Can't connect socket: %s (%d)\n", strerror( errno ), errno );
			throw std::runtime_error("Can't connect socket");
		}

		start();

//  		Console console;
  //		console.addItem( "start", start );
  	//	console.run("inman>");

	}
	catch(const std::exception& error)
	{
		fprintf(stderr, "Fatal error: %s\n", error.what() );
	}

	g_socket.Close();

	exit(0);
}