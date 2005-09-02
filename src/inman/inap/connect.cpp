static char const ident[] = "$Id$";
#include <assert.h>

#include "connect.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

Connect::Connect(Socket* sock) : socket( sock )
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

}

} // namespace inap
} // namespace inmgr
} // namespace smsc
