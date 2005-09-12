#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_DISPATCHER__
#define __SMSC_INMAN_INAP_DISPATCHER__

#include <map>
#include <vector>
#include <queue>

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

class Dispatcher : public Thread
{
	public:

		Dispatcher();
		virtual ~Dispatcher();

		virtual void Run();
		virtual void Stop();
		virtual int  Execute();


	protected:
		Event			   started;
		Event			   stopped;
		volatile bool	   running;
		Logger*	   		   logger;
};

}
}
}

#endif
