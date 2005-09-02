#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#include "core/network/Socket.hpp"

using smsc::core::network::Socket;

static const int SOCKET_TIMEOUT = 1000;


int main(char* argv[], int argc)
{
	if( argc != 3 )
	{
		fprintf( stderr, "Usage: %s <host> <port>", argv[0] );
		exit(1);
	}

	const char* host = argv[1];
	int port = atoi( argv[2]);

	fprintf( stdout, "Connecting to IN manager at %s:%d...", host, port );
	
	Socket socket;

	try
	{
		if( !socket.Init( host, port, SOCKET_TIMEOUT ) )
		{
			throw std::runtime_error("Can't init socket");
		}

		if( !socket.Connect() )
		{
			throw std::runtime_error("Can't init socket");
		}

	}
	catch(const std::exception& error)
	{
		fprintf(stderr, "Fatal error: %s", error.what() );
	}

	socket.Close();

	exit(0);
}