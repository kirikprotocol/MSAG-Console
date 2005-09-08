static char const ident[] = "$Id$";
#include <assert.h>
#include <sys/select.h> 

#include "ss7tmc.h"

#include "dispatcher.hpp"
#include "inman/common/util.hpp"
#include "inman/common/errors.hpp"
#include "factory.hpp"

using smsc::inman::common::dump;
using smsc::inman::common::format;
using smsc::inman::common::getReturnCodeDescription;
using smsc::inman::common::SystemError;
using std::vector;
using std::runtime_error;
using smsc::inman::inap::Dispatcher;

namespace smsc  {
namespace inman {
namespace inap  {

extern Logger* inapLogger;

extern "C" void Dispatcher_Callback(int descriptor, void* uref, int event)
{
	smsc_log_debug(inapLogger,"Dispatcher_Callback called. Handle: 0x%X", descriptor);
	Dispatcher* disp = static_cast<Dispatcher*>(uref);
	disp->fireEvent( descriptor, event );
}

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

	USHORT_T result = EINSS7CpMsgSetEventsExt( handle, MSG_EVENT_READ, Dispatcher_Callback, this  );

	if (result != 0 )
	{
          throw runtime_error( format( "MsgSetEventsExt failed with code %d (%s)", result, getReturnCodeDescription(result)) );
    }
    else
    {
    	smsc_log_debug(logger,"SocketListener registered. Handle: 0x%X", handle);
    }
}

void Dispatcher::removeListener(SocketListener* listener)
{
	assert( listener );
	SOCKET handle = listener->getHandle();
	assert( handle > 0 );

	/* Following function return "unimplemented" error in current ss7 stack impl.
	USHORT_T result = EINSS7CpMsgRemoveEventsExt( handle );
	if (result != 0 )
	{
          smsc_log_error( logger, "MsgRemoveEventsExt failed with code %d (%s)", result, getReturnCodeDescription(result) );
    }
    else
    {
    	smsc_log_debug(logger,"SocketListener unregistered. Handle: 0x%X", handle);
    }
    */
    
	listeners.erase( handle );
}

void Dispatcher::updateAppEvents()
{
	int i = 0;
	appEvents.resize( listeners.size() );
	for( SocketListenersMap::iterator it = listeners.begin(); it != listeners.end(); it++ )
	{
		APP_EVENT_T& appEvent = appEvents[i++];
		appEvent.descriptor = (*it).first;
		appEvent.events     = MSG_EVENT_READ;
		appEvent.callback   = Dispatcher_Callback;
		appEvent.uref 	    = this;
	}
}

void Dispatcher::run()
{
	running = true;
	while( running )
	{
		MSG_T msg;
		memset( &msg, 0, sizeof( MSG_T ));
		msg.receiver = MSG_USER_ID;

		updateAppEvents();
		int len = appEvents.size();
		USHORT_T result = MsgRecvEvent(&msg, &appEvents[0], &len, MSG_INFTIM);
		
		if( MSG_TIMEOUT == result)
		{
			smsc_log_debug(logger,"MsgRecv timeout");
			continue;
		}

		if( MSG_APPL_EVENT == result )
		{
			smsc_log_debug(logger,"Occured %d application even(s)", len);
			for( int i = 0; i < len; i++ )
			{
				APP_EVENT_T& appEvent = appEvents[i];
				appEvent.callback( appEvent.descriptor, appEvent.uref, appEvent.events );
				//fireEvent( appEvent.descriptor, appEvent.events );
			}
			continue;
		}

		if(( MSG_QUEUE_NOT_OPEN == result ) && !running)
		{
			smsc_log_debug(logger,"Dispatcher stopped.");
			continue;
		}

		if (result != 0 )
		{
          	throw runtime_error( format( "MsgRecv failed with code %d (%s)", result, getReturnCodeDescription(result)) );
        }
        else
        {
			smsc_log_debug(logger,"Message received:");
			smsc_log_debug(logger," sender=0x%X", msg.sender);
			smsc_log_debug(logger," receiver=0x%X", msg.receiver);
			smsc_log_debug(logger," primitive=0x%X", msg.primitive);
			smsc_log_debug(logger," data=%s", dump(msg.size,msg.msg_p).c_str());
        }

        EINSS7_I97THandleInd(&msg);
      	EINSS7CpReleaseMsgBuffer(&msg);
/*
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

  		if( n < 0 )	throw SystemError("select failed");

  		smsc_log_debug(logger, "%d socket(s) has data", n );

  		int j = 0;

		for( SocketListenersMap::iterator i = listeners.begin(); i != listeners.end(); i++ )
		{
			SOCKET handle = (*i).first;
			if( FD_ISSET( handle, &handlesBitmap ) )
			{
				SocketListener* listener = (*i).second;
				assert( listener );
				smsc_log_debug(logger, "Handle data on socket 0x%X", handle );
				listener->process( this );
				j++;
			}
		}
		assert( j = n );
 */
	}
}

void Dispatcher::Stop() 
{
	running = false;
	close( EINSS7CpMsgObtainSocket(MSG_USER_ID, TCAP_ID) ); // UGLY !
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

void Dispatcher::fireEvent(int handle, int event)
{
	smsc_log_debug(logger, "Event 0x%X from socket 0x%X", event, handle );
	SocketListenersMap::iterator i = listeners.find( handle );
	if( i == listeners.end() )
		throw runtime_error( format("Invalid handle: 0x%X", handle) );
	SocketListener* listener = 	(*i).second;
	listener->process( this );
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
