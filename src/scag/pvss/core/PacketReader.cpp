#include "PacketReader.h"

namespace scag2 {
namespace pvss {
namespace core {

bool PacketReader::setupSocket( PvssSocket& con )
{
    mul_.add( con.socket() );
    return true;
}

} // namespace core
} // namespace pvss
} // namespace scag2
