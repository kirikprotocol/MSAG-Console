#include "PacketWriter.h"
#include "Config.h"
#include "Core.h"
#include "scag/util/HexDump.h"

namespace scag2 {
namespace pvss {
namespace core {

void PacketWriter::serialize( const Packet& packet, Protocol::Buffer& buffer ) throw (PvssException)
{
    Protocol& proto = *core_->getProtocol();
    buffer.SetPos(4);
    proto.serialize(packet,buffer);
    uint32_t buflen = buffer.GetPos()-4;
    if ( buflen > getConfig().getPacketSizeLimit() )
        throw PvssException( PvssException::IO_ERROR, "Illegal serialized packet size=%d",buflen);
    buflen = htonl(buflen);
    memcpy(buffer.get(),reinterpret_cast<char*>(&buflen),4);
}

} // namespace core
} // namespace pvss
} // namespace scag2
