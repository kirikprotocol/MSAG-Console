#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PACKET_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PACKET_HPP__

# include <sys/types.h>

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

struct Packet {
  Packet()
    : packet_data_len(0), packet_type(0), protocol_family(0) {}

  static const size_t MAX_PACKET_LEN = 65536;

  // contains all data got from network
  uint8_t packet_data[MAX_PACKET_LEN];
  size_t packet_data_len;
  uint32_t packet_type;
  uint8_t protocol_family;
  size_t addValue(uint8_t value);
  size_t addValue(uint16_t value);
  size_t addValue(uint32_t value);

  size_t addValue(const uint8_t* value, size_t value_length);

  size_t extractValue(uint8_t* value, size_t offset) const;
  size_t extractValue(uint16_t* value, size_t offset) const;
  size_t extractValue(uint32_t* value, size_t offset) const;

  bool isSufficientSpace(size_t offset, size_t required_size) const;
};

}}}

#endif
