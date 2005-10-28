static char const ident[] = "$Id$";
#include <assert.h>

#include "inman/interaction/connect.hpp"
#include "inman/interaction/serializer.hpp"

using smsc::inman::interaction::SerializerITF;
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObject;
//using smsc::inman::common::dump;

namespace smsc  {
namespace inman {
namespace interaction {

Connect::Connect(Socket* sock, SerializerITF * serializer)
    : logger(Logger::getInstance("smsc.inman.inap.Connect"))
    , socket( sock )
    , pipe( new ObjectPipe( sock, serializer ) )
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
    SerializableObject* cmd = pipe->receive();
	
    if (!cmd)
	return false;

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

} // namespace interaction
} // namespace inman
} // namespace smsc

