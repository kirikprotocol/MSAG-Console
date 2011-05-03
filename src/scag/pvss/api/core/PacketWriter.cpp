#include "PacketWriter.h"
#include "Config.h"
#include "Core.h"
#include "scag/util/io/HexDump.h"

namespace scag2 {
namespace pvss {
namespace core {

void PacketWriter::serialize( const Packet& packet, Protocol::Buffer& buffer ) /* throw (PvssException) */ 
{
    Protocol& proto = *core_->getProtocol();
    buffer.SetPos(4);
    proto.serialize(packet,buffer);
    uint32_t buflen = uint32_t(buffer.GetPos()-4);
    if ( buflen > getConfig().getPacketSizeLimit() )
        throw PvssException( PvssException::IO_ERROR, "Illegal serialized packet size=%d",buflen);
    buflen = htonl(buflen);
    memcpy(buffer.get(),reinterpret_cast<char*>(&buflen),4);
}


void PacketWriter::writePending()
{
    if ( writePending_ ) return;
    smsc::core::synchronization::MutexGuard mg(mon_);
    if ( writePending_ ) return;
    writePending_ = true;
    mon_.notify();
    while ( sockets_.Count() > 0 && writePending_ ) {
        mon_.wait(200);
    }
    return;
}


void PacketWriter::setupFailed(util::msectime_type currentTime)
{
    if ( writePending_ ) {
        writePending_ = false;
        mon_.notify();
    } else {
        IOTask::setupFailed(currentTime);
    }
}

} // namespace core
} // namespace pvss
} // namespace scag2
