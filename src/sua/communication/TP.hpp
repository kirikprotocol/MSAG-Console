#ifndef IO_DISPATCHER_TP_HPP_HEADER_INCLUDED_B87B0D8B
# define IO_DISPATCHER_TP_HPP_HEADER_INCLUDED_B87B0D8B 1

# include <sys/types.h>

namespace communication {

struct TP {
  TP()
    : packetType(0), packetLen(0), packetBody(packetBodyMem) {}

  TP(int aPacketType, size_t aPacketLen, uint8_t* packetBodyPtr)
    : packetType(aPacketType), packetLen(aPacketLen), packetBody(packetBodyPtr) {}

  int packetType; // class of transport packet contained message, eg. packet contained SUA messages, or packet contained Libsua messages.

  size_t packetLen; // length of data in packetBody array
  enum { MAX_PACKET_SIZE = 64*1024 };
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

} // namespace io_dispatcher

#include <sua/utilx/PreallocatedMemoryManager.hpp>

namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_PACKET>() {
  static TSD_Init_Helper<MEM_FOR_PACKET> tsd_init;
  static communication::TP tpObjectToGetItsSize;
  alloc_mem_desc_t* res = allocateMemory(sizeof(tpObjectToGetItsSize), tsd_init._tsd_memory_key);

  return res;
}

}

#endif /* IO_DISPATCHER_TP_HPP_HEADER_INCLUDED_B87B0D8B */
