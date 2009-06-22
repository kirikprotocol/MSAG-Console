#include "SocketReader.h"
#include "Core.h"
#include "Config.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

bool SocketReader::setupSocket( Socket& socket, util::msectime_type )
{
    mul_.add( socket.socket() );
    return true;
}


bool SocketReader::hasEvents()
{
    return mul_.canRead( ready_, error_, core_.getConfig().getIOTimeout() );
}


void SocketReader::process( Socket& socket )
{
    socket.processInput();
}

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2
