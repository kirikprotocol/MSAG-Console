#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPPACKETFORMATTER_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPPACKETFORMATTER_HPP__

# include "logger/Logger.h"
# include "eyeline/load_balancer/io_subsystem/PacketFormattingStrategy.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SMPPPacketFormatter : public io_subsystem::PacketFormattingStrategy {
public:
  SMPPPacketFormatter();
  virtual ~SMPPPacketFormatter();
protected:
  virtual void readNextPartOfPacketHeader(corex::io::InputStream* iStream);
  virtual void readNextPartOfPacketBody(corex::io::InputStream* iStream);
  virtual void readNextPartOfPacketTailer(corex::io::InputStream* iStream);
  virtual bool hasBeenPacketHeaderReadCompletely() const;
  virtual bool hasBeenPacketBodyReadCompletely() const;
  virtual bool hasBeenPacketTailerReadCompletely() const;
  virtual bool hasBeenTotalPacketReadCompletely() const;

  virtual io_subsystem::Packet* getCompletePacket();
private:
  io_subsystem::Packet* _packet;
  size_t _offset, _pduSize;
  smsc::logger::Logger* _logger;
};

}}}}

#endif
