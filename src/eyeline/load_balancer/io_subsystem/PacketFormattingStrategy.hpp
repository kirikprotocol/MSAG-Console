#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PACKETFORMATTINGSTRATEGY_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PACKETFORMATTINGSTRATEGY_HPP__

# include <sys/types.h>
# include <string>

# include "eyeline/corex/io/IOStreams.hpp"
# include "eyeline/load_balancer/io_subsystem/Packet.hpp"
# include "eyeline/load_balancer/io_subsystem/Message.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class PacketFormattingStrategy {
public:
  virtual ~PacketFormattingStrategy() {}

  Packet* receive(corex::io::InputStream* i_stream);
  virtual Packet* prepareToSend(const Message& msg);
protected:
  virtual void readNextPartOfPacketHeader(corex::io::InputStream* i_stream) = 0;
  virtual void readNextPartOfPacketBody(corex::io::InputStream* i_stream) = 0;
  virtual void readNextPartOfPacketTailer(corex::io::InputStream* i_stream) = 0;
  virtual bool hasBeenPacketHeaderReadCompletely() const = 0;
  virtual bool hasBeenPacketBodyReadCompletely() const = 0;
  virtual bool hasBeenPacketTailerReadCompletely() const = 0;
  virtual bool hasBeenTotalPacketReadCompletely() const = 0;
  virtual Packet* getCompletePacket() = 0;
};

}}}

#endif

