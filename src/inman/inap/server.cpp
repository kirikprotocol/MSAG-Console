static char const ident[] = "$Id$";
#include <assert.h>

#include "server.hpp"
#include "inman/common/util.hpp"
#include "inman/common/errors.hpp"

using smsc::inman::common::format;
using smsc::inman::common::SystemError;

namespace smsc  {
namespace inman {
namespace inap  {

Server::Server(const char* host, int port)
	: logger(Logger::getInstance("smsc.inman.inap.Server"))
	, running( false )
{
  	if( serverSocket.InitServer( host, port, 0 ) != 0 )
  	{
  		throw SystemError( format("Can't init server socket. Host: %s:%d", host, port)  );
  	}

  	if( serverSocket.StartServer() != 0 )
  	{
  		throw SystemError( format("Can't start server socket. Host: %s:%d", host, port)  );
  	}
}

Server::~Server()
{
	serverSocket.Close();
}

void Server::openConnect(Connect* connect)
{
	assert( connect );
	connects.push_back( connect );
}

void Server::closeConnect(Connect* connect)
{
	assert( connect );
	connects.remove( connect );
	delete connect;
}

void Server::Run()
{
	running = true;
	while( running )
	{
			fd_set read;
			fd_set error;
			
			FD_ZERO( &read );
			FD_ZERO( &error );

			FD_SET( serverSocket.getSocket(), &read );
			FD_SET( serverSocket.getSocket(), &error );

			int max = serverSocket.getSocket();

			for( Connects::iterator i = connects.begin(); i != connects.end(); i++ )
			{
				SOCKET socket = (*i)->getSocket()->getSocket();
				FD_SET( socket, &read );
				FD_SET( socket, &error );
				if( socket > max ) max = socket;
			}

  			int n  = select(  max+1, &read, 0, &error, 0 );

  			if( n < 0 )
  			{
  				if( !running )
  				{
  					smsc_log_debug(logger, "Server stopped");
  					continue;
  				}
  				throw SystemError("select failed");
  			}

			if( FD_ISSET( serverSocket.getSocket(), &read ) )
			{
				Socket* clientSocket = serverSocket.Accept();
				if( !clientSocket )
				{
					smsc_log_error(logger, "Can't accept client connection %d (%s)", errno, strerror( errno ) );
				}
				else
				{
					smsc_log_debug(logger, "Open new connect (0x%X)", clientSocket->getSocket());
					Connect* connect = new Connect( clientSocket );
			    	openConnect( connect );
					continue;
				}
			}

			if( FD_ISSET( serverSocket.getSocket(), &error ) )
			{
				smsc_log_debug(logger, "Server socket error - exit." );
				running = false;
				continue;
			}

			// Iterate over copy for safe modify original collection
			Connects stump( connects );
			for( Connects::iterator i = stump.begin(); i != stump.end(); i++ )
			{
				Connect* conn = (*i);
				SOCKET socket = conn->getSocket()->getSocket();

				if( FD_ISSET( socket, &read ) )
				{
					smsc_log_debug(logger, "Event on socket 0x%X", socket );

					if( !conn->process( this ) )
					{
						smsc_log_debug(logger, "Close socket 0x%X", socket );
						closeConnect( conn );
					}
				}

				if( FD_ISSET( socket, &error ) )
				{
					smsc_log_debug(logger, "Error - close socket (0x%X)", socket );
					closeConnect( conn );
				}
			}
	}
}

void Server::Stop() 
{
	running = false;
	serverSocket.Close();
}

// Thread entry point
int Server::Execute()
{
	int result = 0;
  	started.SignalAll();
	try
	{
      	smsc_log_debug(logger, "Thread started");
		Run();
	}
	catch(const std::exception& error)
	{
      	smsc_log_error(logger, "Error in thread: %s", error.what() );
      	result = 1;
	}
  	stopped.SignalAll();
	smsc_log_debug(logger, "Thread finished");
  	return result;
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
