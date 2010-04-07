#ifndef __EYELINE_SS7NA_COMMON_TP_HPP__
# define __EYELINE_SS7NA_COMMON_TP_HPP__

# include <sys/types.h>

namespace eyeline {
namespace ss7na {
namespace common {

struct TP {
  enum { MAX_PACKET_SIZE = 64*1024 };

  TP()
    : packetType(0), packetLen(0), maxPacketLen(MAX_PACKET_SIZE), packetBody(packetBodyMem) {}

  TP(unsigned packet_type, size_t packet_len, uint8_t* packet_body_ptr, size_t max_packet_len)
    : packetType(packet_type), packetLen(packet_len), maxPacketLen(max_packet_len),
      packetBody(packet_body_ptr) {}

  unsigned packetType;
  unsigned protocolClass;

  size_t packetLen; // length of data in packetBody array
  size_t maxPacketLen; // max possible size of data in packetBody array

  uint8_t* packetBody;
  uint8_t packetBodyMem[MAX_PACKET_SIZE]; // contains all packet data including length field

  struct packet_sctp_properties {
    packet_sctp_properties()
      : streamNo(0), orderingTransfer(false) {}
      
    uint16_t streamNo;
    bool orderingTransfer;
  };
  packet_sctp_properties pkt_sctp_props;
};

size_t addField(TP* packet, size_t position, uint8_t val /*in host byteorder*/);
size_t addField(TP* packet, size_t position, uint16_t val /*in host byteorder*/);
size_t addField(TP* packet, size_t position, uint32_t val /*in host byteorder*/);
size_t addField(TP* packet, size_t position, const uint8_t* val, size_t valSz);

size_t extractField(const TP& packetBuf, size_t position, uint8_t* fieldVal /*return value in host byteorder*/);
size_t extractField(const TP& packetBuf, size_t position, uint16_t* fieldVal /*return value in host byteorder*/);
size_t extractField(const TP& packetBuf, size_t position, uint32_t* fieldVal /*return value in host byteorder*/);
size_t extractField(const TP& packetBuf, size_t position, uint8_t* fieldVal, size_t fieldValSz);

}}}

#include "eyeline/utilx/PreallocatedMemoryManager.hpp"

namespace eyeline {
namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_PACKET>() {
  static TSD_Init_Helper<MEM_FOR_PACKET> tsd_init;
  static ss7na::common::TP tpObjectToGetItsSize;
  alloc_mem_desc_t* res = allocateMemory(sizeof(tpObjectToGetItsSize), tsd_init._tsd_memory_key);

  return res;
}

}}

#endif
