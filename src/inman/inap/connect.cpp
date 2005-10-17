static char const ident[] = "$Id$";
#include <assert.h>

#include "connect.hpp"
#include "server.hpp"

#include "inman/common/util.hpp"
#include "inman/interaction/serializer.hpp"
#include "inman/interaction/messages.hpp"

using smsc::inman::interaction::SerializerInap;
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObject;
using smsc::inman::interaction::InmanCommand;

using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::DeliverySmsResult;

using smsc::inman::common::dump;

namespace smsc  {
namespace inman {
namespace inap  {

Connect::Connect(Socket* sock) 
	: logger(Logger::getInstance("smsc.inman.inap.Connect"))
	, socket( sock )
	, pipe( new ObjectPipe( sock, SerializerInap::getInstance() ) )
{
		
	assert( socket );
}

Connect::~Connect()
{
	delete pipe;
	delete socket;
}

Socket* Connect::getSocket()
{
	return socket;
}

bool Connect::process()
{
	InmanCommand* cmd = static_cast<InmanCommand*>(pipe->receive());
	
	if( !cmd )
	{
		return false;
	}

	for( ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++)
	{
		ConnectListener* ptr = *it;
		ptr->onCommandReceived( this, cmd );
	}

	delete cmd;

	return true;
}

void Connect::send(SerializableObject* obj)
{
	assert( obj );
	assert( pipe );
	pipe->send( obj );	
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
