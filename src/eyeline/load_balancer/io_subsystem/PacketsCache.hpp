#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PACKETSCACHE_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PACKETSCACHE_HPP__

# include <map>
# include <list>
# include <sys/types.h>

# include "logger/Logger.h"
# include "eyeline/corex/io/IOStreams.hpp"
# include "eyeline/load_balancer/io_subsystem/Packet.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class PacketsCache {
public:
  PacketsCache()
  : _logger(smsc::logger::Logger::getInstance("io_subsystem"))
  {}

  struct cache_element {
    explicit cache_element(const Packet* aPacket)
      : offset(0), packet(aPacket)
    {}
    const Packet* packet;
    size_t offset;
  };

  void storePacketInCache(corex::io::OutputStream* o_stream, const Packet* packet);
  cache_element getPacket(corex::io::OutputStream* o_stream);
  // return true if there isn't any more packet in cache for this ostream
  bool commitWrittenPacketBytes(corex::io::OutputStream* o_stream, ssize_t written_bytes);
  void removePacketsFromCache(corex::io::OutputStream* o_stream);
private:
  smsc::logger::Logger* _logger;
  typedef std::list<cache_element> packets_t;
  typedef std::map<corex::io::OutputStream*, packets_t> packets_cache_t;
  packets_cache_t _packetsCache;
};

}}}

#endif
