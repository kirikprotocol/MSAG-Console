#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#include "core/network/Socket.hpp"

using smsc::core::network::Socket;

static const int SOCKET_TIMEOUT = 1000;


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
	
	Socket socket;

	try
	{
		if( socket.Init( host, port, SOCKET_TIMEOUT ) != 0 )
		{
			fprintf( stderr, "Can't init socket: %s (%d)\n", strerror( errno ), errno );
			throw std::runtime_error("Can't init socket");
		}

		if( socket.Connect() != 0 )
		{
			fprintf( stderr, "Can't connect socket: %s (%d)\n", strerror( errno ), errno );
			throw std::runtime_error("Can't connect socket");
		}

		for( int i = 1; i < 100; i++ )
		{
			socket.Printf("Message #%d from client", i);
			usleep( 1000 * 1000 );
	    }

	}
	catch(const std::exception& error)
	{
		fprintf(stderr, "Fatal error: %s\n", error.what() );
	}

	socket.Close();

	exit(0);
}