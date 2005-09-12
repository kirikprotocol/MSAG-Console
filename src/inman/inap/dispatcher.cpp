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

		USHORT_T result = MsgRecvEvent(&msg, NULL, NULL, MSG_INFTIM);
		
		if( MSG_TIMEOUT == result)
		{
			smsc_log_debug(logger,"MsgRecv timeout");
			continue;
		}

		if(( MSG_QUEUE_NOT_OPEN == result ) && !running)
		{
			smsc_log_debug(logger,"SS7 stack stopped");
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
