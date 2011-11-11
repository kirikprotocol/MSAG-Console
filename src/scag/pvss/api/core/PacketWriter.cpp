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
    if ( buflen > unsigned(getConfig().getPacketSizeLimit()) )
        throw PvssException( PvssException::IO_ERROR, "Illegal serialized packet size=%d",buflen);
    buflen = htonl(buflen);
    memcpy(buffer.get(),reinterpret_cast<char*>(&buflen),4);
}


void PacketWriter::writePending()
{
    if ( writePending_ ) return;
    smsc::core::synchronization::MutexGuard mg(pmon_);
    if ( writePending_ ) return;
    if ( isReleased ) return;
    writePending_ = true;
    pmon_.notify();
    while ( writePending_ && !isReleased ) {
        pmon_.wait(200);
    }
    return;
}


int PacketWriter::setupFailed(int tmo)
{
    smsc::core::synchronization::MutexGuard mg(pmon_);
    if ( writePending_ ) {
        writePending_ = false;
        pmon_.notify();
        return 0;
    } else {
        return IOTask::setupFailed(tmo);
    }
}

} // namespace core
} // namespace pvss
} // namespace scag2
