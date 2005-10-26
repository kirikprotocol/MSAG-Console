static char const ident[] = "$Id$";

#include <string>
#include <stdexcept>

#include "inman/common/synch.hpp"
#include "inman/common/util.hpp"
#include "inman/inap/inss7util.hpp"
#include "inman/inap/dispatcher.hpp"

using std::runtime_error;

using smsc::inman::common::Synch;
using smsc::inman::common::format;


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

void Dispatcher::Run()
{
	running = true;
	while( running )
	{
		MSG_T msg;
		memset( &msg, 0, sizeof( MSG_T ));
		msg.receiver = MSG_USER_ID;

		Synch::getInstance()->Unlock();
		USHORT_T result = MsgRecvEvent(&msg, NULL, NULL, /*MSG_INFTIM*/ 100 );
		if( !running ) break;
		Synch::getInstance()->Lock();
		
		if( MSG_TIMEOUT == result)
		{
			continue;
		}

		if (result != 0 )
		{
         	throw runtime_error( format( "MsgRecv failed with code %d (%s)", result, getReturnCodeDescription(result)) );
        }

		EINSS7_I97THandleInd(&msg);
		EINSS7CpReleaseMsgBuffer(&msg);
	}
	smsc_log_debug(logger,"SS7 stack stopped");
}

void Dispatcher::Stop() 
{
	running = false;
}

// Thread entry point
int Dispatcher::Execute()
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

