#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPMESSAGE_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPMESSAGE_HPP__

# include "eyeline/load_balancer/io_subsystem/Message.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SMPPMessage : public io_subsystem::Message {
public:
  SMPPMessage(uint32_t commandId);

  virtual message_code_t getMsgCode() const;
  virtual size_t serialize(io_subsystem::Packet* packet) const;
  virtual size_t deserialize(const io_subsystem::Packet* packet);

  virtual uint32_t getCommandLength() const;

  uint32_t getCommandStatus() const;
  void setCommandStatus(uint32_t commandStatus);

  uint32_t getSequenceNumber() const;
  void setSequenceNumber(uint32_t sequenceNumber);

  virtual std::string toString() const;

  enum { SMPP_HEADER_SZ = 16 };

protected:
  void setFinalDate(const char* finalDate);
  void setMessageState(uint8_t messageState);
  void setErrorCode(uint8_t errorCode);

  virtual uint32_t calculateCommandBodyLength() const { return 0; }

private:
  uint32_t _commandLength, _commandId, _commandStatus, _sequenceNumber;
  bool _isSetCommandLength, _isSetCommandId, _isSetCommandStatus, _isSetSequenceNumber;
};

enum {
  ESME_ROK = 0, ESME_RALYBND = 0x00000005, ESME_RSYSERR = 0x00000008,
  ESME_RINVMSGID = 0x0000000C, ESME_RBINDFAIL = 0x0000000D, ESME_RSUBMITFAIL = 0x00000045,
  ESME_RINVOPTPARAMVAL = 0x000000C4
};

}}}}

#include <eyeline/utilx/PreallocatedMemoryManager.hpp>

namespace eyeline {
namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_MESSAGE>() {
  static TSD_Init_Helper<MEM_FOR_MESSAGE> tsd_init;
  static const size_t MAX_MESSAGE_SIZE = 128 * 1024;// 128 KB
  alloc_mem_desc_t* res = allocateMemory(MAX_MESSAGE_SIZE, tsd_init._tsd_memory_key);

  return res;
}

}}

#endif
