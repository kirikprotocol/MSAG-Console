static char const ident[] = "$Id$";
#include <assert.h>

#include "connect.hpp"

#include "inman/common/util.hpp"
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

void   Connect::process(Dispatcher*)
{
	char buf[1024];
  	int n = socket->Read(buf, sizeof(buf) - 1);
	smsc_log_debug(logger, "Received: %s", dump( n, (unsigned char*)buf, true ).c_str() );
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
