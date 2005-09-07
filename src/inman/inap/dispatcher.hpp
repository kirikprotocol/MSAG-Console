#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_DISPATCHER__
#define __SMSC_INMAN_INAP_DISPATCHER__

#include <map>
#include <vector>

#include "ss7cp.h"

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"

using smsc::logger::Logger;
using smsc::core::network::Socket;
using smsc::core::threads::Thread;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;

namespace smsc  {
namespace inman {
namespace inap  {

class Dispatcher;

class SocketListener
{
public:
	virtual SOCKET getHandle()  = 0;
	virtual void   process(Dispatcher*)  = 0;
};

class Dispatcher : public Thread
{
		typedef std::map<SOCKET, SocketListener*> SocketListenersMap;
		typedef std::vector< APP_EVENT_T > 		  AppEventVector;

	public:

		Dispatcher();
		virtual ~Dispatcher();

		virtual void addListener(SocketListener*);
		virtual void removeListener(SocketListener*);

		virtual void run();

		virtual void Stop();

		virtual int  Execute();

		virtual void fireEvent(int handle, int event);

	protected:

		virtual void updateAppEvents();

		SocketListenersMap listeners;
		AppEventVector	   appEvents;
		Event			   started;
		Event			   stopped;
		volatile bool	   running;
		Logger*	   		   logger;
};

}
}
}

#endif
