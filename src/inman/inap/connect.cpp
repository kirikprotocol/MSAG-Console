static char const ident[] = "$Id$";
#include <assert.h>

#include "connect.hpp"
#include "server.hpp"

#include "inman/common/util.hpp"
#include "inman/interaction/serializer.hpp"
#include "inman/interaction/messages.hpp"

using smsc::inman::interaction::Serializer;
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
	, pipe( new ObjectPipe( sock ) )
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

bool Connect::process(Server* pServer)
{
	InmanCommand* obj = static_cast<InmanCommand*>(pipe->receive());
	assert( obj );
	obj->handle( this );
	return true;
}

void Connect::onChargeSms(ChargeSms* sms)
{
	assert( sms );
	smsc_log_debug(logger, "ChargeSms received");
}

void Connect::onDeliverySmsResult(DeliverySmsResult* sms)
{
	smsc_log_debug(logger, "DeliverySmsResult received");
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
