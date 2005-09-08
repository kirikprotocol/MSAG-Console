static char const ident[] = "$Id$";
#include <assert.h>

#include "connect.hpp"

#include "inman/common/util.hpp"
#include "inman/interaction/serializer.hpp"

using smsc::inman::interaction::Serializer;
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObject;

using smsc::inman::common::dump;

namespace smsc  {
namespace inman {
namespace inap  {

Connect::Connect(Socket* sock) 
	: logger(Logger::getInstance("smsc.inman.inap.Connect"))
	, socket( sock )
{
	assert( socket );
}

Connect::~Connect()
{
	delete socket;
}

SOCKET Connect::getHandle()
{
	return socket->getSocket();
}

void   Connect::process(Dispatcher* pDisp)
{
	char buf[1024]; //todo: read all data, that may be more then 1024

  	int n = socket->Read(buf, sizeof(buf) - 1);
  	if( n < 1 )
  	{
  		smsc_log_debug(logger, "Socket closed - killing connection");
  		pDisp->removeListener( this );
  		delete this; // shit! 
  		return;
  	}

  	ObjectBuffer buffer( n );
  	buffer.Append( buf, n );
  	buffer.SetPos( 0 );

	smsc_log_debug(logger, "Received: %s", dump( n, (unsigned char*)buf ).c_str() );

	SerializableObject* obj = Serializer::getInstance()->deserialize( buffer );
	assert( obj );
	obj->run();

}

} // namespace inap
} // namespace inmgr
} // namespace smsc
