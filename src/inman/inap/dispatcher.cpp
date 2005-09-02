static char const ident[] = "$Id$";
#include <assert.h>
#include <sys/select.h> 

#include "dispatcher.hpp"
#include "inman/common/errors.hpp"

using smsc::inman::common::SystemError;

namespace smsc  {
namespace inman {
namespace inap  {

Dispatcher::Dispatcher() 
	: logger(Logger::getInstance("smsc.inman.inap.Dispatcher"))
	, running( false )
{
}

Dispatcher::~Dispatcher()
{
}

void Dispatcher::addListener(SocketListener* listener)
{
	assert( listener );
	SOCKET handle = listener->getHandle();
	assert( handle > 0 );
	listeners.insert( SocketListenersMap::value_type( handle, listener ) );
}

void Dispatcher::removeListener(SocketListener* listener)
{
	assert( listener );
	SOCKET handle = listener->getHandle();
	assert( handle > 0 );
	listeners.erase( handle );
}

// Main loop
void Dispatcher::run()
{
	running = true;
	while( running )
	{
		int max = 0;

		fd_set handlesBitmap;
		
		FD_ZERO(&handlesBitmap);

		for( SocketListenersMap::iterator i = listeners.begin(); i != listeners.end(); i++ )
		{
			SOCKET handle = (*i).first;
			FD_SET( handle, &handlesBitmap );
			if( handle > max ) max = handle;
		}

  		int n  = select(  max + 1, &handlesBitmap, 0, 0, 0 );

  		if( n < 0 )	throw new SystemError("select failed");

  		int j = 0;

		for( SocketListenersMap::iterator i = listeners.begin(); i != listeners.end(); i++ )
		{
			SOCKET handle = (*i).first;
			if( FD_ISSET( handle, &handlesBitmap ) )
			{
				SocketListener* listener = (*i).second;
				assert( listener );
				smsc_log_debug(logger, "Handle data on socket 0x%04X", handle );
				listener->process( this );
				j++;
			}
		}
		assert( j = n );
	}
}
 
// Thread entry point
int Dispatcher::Execute()
{
	int result = 0;
  	started.SignalAll();
	try
	{
      	smsc_log_debug(logger, "Thread started");
		run();
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
