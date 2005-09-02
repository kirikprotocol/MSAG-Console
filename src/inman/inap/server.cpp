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
{
  	if( socket.InitServer( host, port, 0 ) != 0 )
  	{
  		throw SystemError( format("Can't init server socket. Host: %s:%d", host, port)  );
  	}

  	if( socket.StartServer() != 0 )
  	{
  		throw SystemError( format("Can't start server socket. Host: %s:%d", host, port)  );
  	}
}

Server::~Server()
{
	socket.Close();
}

void Server::process(Dispatcher* disp)
{
	Socket *clientSocket = socket.Accept();
	if( !clientSocket )
	{
		smsc_log_error(logger, "Can't accept client connection %d (%s)", errno, strerror( errno ) );
	}
	else
	{
		smsc_log_debug(logger, "Accepting new client connection");
		Connect* conn = new Connect( clientSocket );
		disp->addListener( conn );
	}
}

SOCKET Server::getHandle()
{
	return socket.getSocket();
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
