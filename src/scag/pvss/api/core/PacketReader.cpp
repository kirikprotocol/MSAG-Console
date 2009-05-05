#include "PacketReader.h"
#include "Config.h"

namespace scag2 {
namespace pvss {
namespace core {

bool PacketReader::setupSocket( PvssSocket& con, util::msectime_type )
{
    mul_.add( con.socket() );
    return true;
}

bool PacketReader::hasEvents() 
{
    return mul_.canRead(ready_, error_, config_->getIOTimeout()); 
}

} // namespace core
} // namespace pvss
} // namespace scag2
