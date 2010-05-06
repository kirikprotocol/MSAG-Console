#include <utility>
#include "PacketsCache.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

void
PacketsCache::storePacketInCache(corex::io::OutputStream* o_stream, const Packet* packet)
{
  smsc_log_debug(_logger, "PacketsCache::storePacketInCache::: store packet=0x%p with type=0x%08X for socket='%s'",
                 packet, packet->packet_type, o_stream->getOwner()->toString().c_str());
  packets_cache_t::iterator iter = _packetsCache.find(o_stream);
  if ( iter == _packetsCache.end() ) {
    std::pair<packets_cache_t::iterator, bool> insResult = _packetsCache.insert(std::make_pair(o_stream, packets_t()));
    iter = insResult.first;
  }
  packets_t& cachedPackets = iter->second;
  cachedPackets.push_back(cache_element(packet));
}

PacketsCache::cache_element
PacketsCache::getPacket(corex::io::OutputStream* o_stream)
{
  packets_cache_t::iterator iter = _packetsCache.find(o_stream);
  smsc_log_debug(_logger, "PacketsCache::getPacket::: get for socket='%s'",
                 o_stream->getOwner()->toString().c_str());

  if ( iter == _packetsCache.end() )
    throw smsc::util::Exception ("PacketsCache::getPacket::: unknown output stream=[%s]",
                                 o_stream->getOwner()->toString().c_str());
  if ( iter->second.empty() )
    throw smsc::util::Exception ("PacketsCache::getPacket::: there isn't cached packet for output stream=[%s]", o_stream->getOwner()->toString().c_str());

  return iter->second.front();
}

bool
PacketsCache::commitWrittenPacketBytes(corex::io::OutputStream* o_stream, ssize_t written_bytes)
{
  smsc_log_debug(_logger, "PacketsCache::commitWrittenPacketBytes::: commit %d bytes for socket='%s'",
                 written_bytes, o_stream->getOwner()->toString().c_str());

  packets_cache_t::iterator iter = _packetsCache.find(o_stream);
  if ( iter == _packetsCache.end() )
    throw smsc::util::Exception ("PacketsCache::commitWrittenPacketBytes::: unknown output stream=[%s]", o_stream->getOwner()->toString().c_str());

  cache_element& cachedPacket = iter->second.front();
  cachedPacket.offset += written_bytes;
  if (cachedPacket.offset == cachedPacket.packet->packet_data_len) {
    delete cachedPacket.packet;
    iter->second.pop_front();
  }

  if (iter->second.empty())
    return true;

  return false;
}

void
PacketsCache::removePacketsFromCache(corex::io::OutputStream* o_stream)
{
  smsc_log_debug(_logger, "PacketsCache::removePacketsFromCache::: remove all cached packets for socket='%s'",
                 o_stream->getOwner()->toString().c_str());

  packets_cache_t::iterator iter = _packetsCache.find(o_stream);
  if ( iter == _packetsCache.end() )
    return;

  while(!iter->second.empty()) {
    cache_element& cachedPacket = iter->second.front();
    delete cachedPacket.packet;
    iter->second.pop_front();
  }

  _packetsCache.erase(iter);
}

}}}
