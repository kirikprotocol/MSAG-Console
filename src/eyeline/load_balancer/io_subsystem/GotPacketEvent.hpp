#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_GOTPACKETEVENT_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_GOTPACKETEVENT_HPP__

# include "logger/Logger.h"
# include "util/Exception.hpp"
# include "eyeline/load_balancer/io_subsystem/Packet.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/IOEvent.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class GotPacketEvent : public IOEvent {
public:
  GotPacketEvent(const Packet* packet, IOProcessor& io_processor, const LinkId& link_id)
  : _logger(smsc::logger::Logger::getInstance("io_subsystem")),
    _packet(packet), _ioProcessor(io_processor), _linkId(link_id)
  {
    if ( !_packet )
      throw smsc::util::Exception("GotPacketEvent::GotPacketEvent::: packet is NULL");
  }

  virtual ~GotPacketEvent() {
    delete _packet;
  }

  virtual void handle();
private:
  smsc::logger::Logger* _logger;
  const Packet* _packet;
  IOProcessor& _ioProcessor;
  LinkId _linkId;
};

}}}

#endif
