#ifndef __EYELINE_SS7NA_LIBSCCP_MESSAGES_LIBSCCPMESSAGE_HPP__
# define __EYELINE_SS7NA_LIBSCCP_MESSAGES_LIBSCCPMESSAGE_HPP__

# include <sys/types.h>
# include <string>
# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/Message.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class LibsccpMessage : public common::Message {
public:
  LibsccpMessage(uint32_t msgCode);

  virtual size_t serialize(common::TP* result_buf) const;
  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const common::TP& packet_buf);
  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

  virtual std::string toString() const;

  virtual uint32_t getMsgCode() const { return _msgCode; }

  static const size_t MAX_MESSAGE_SIZE = 4096;

protected:
  virtual uint32_t getLength() const { return _msgLen; }

  void setLength(uint32_t msgLen) { _msgLen = msgLen; }

  static const size_t _MSGCODE_SZ = sizeof(uint32_t), _MSGLEN_SZ = sizeof(uint32_t);
private:
  uint32_t _msgLen;
  uint32_t _msgCode;
};

}}}

#include "eyeline/utilx/PreallocatedMemoryManager.hpp"

namespace eyeline {
namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_LIBSCCP_MESSAGE>() {
  static TSD_Init_Helper<MEM_FOR_LIBSCCP_MESSAGE> tsd_init;
  alloc_mem_desc_t* res = allocateMemory(ss7na::libsccp::LibsccpMessage::MAX_MESSAGE_SIZE,
                                         tsd_init._tsd_memory_key);

  return res;
}

}}

#endif
