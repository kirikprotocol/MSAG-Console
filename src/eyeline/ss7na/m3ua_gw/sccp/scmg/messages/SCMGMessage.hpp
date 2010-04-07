#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGES_SCMGMESSAGE_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGES_SCMGMESSAGE_HPP__

# include "eyeline/utilx/types.hpp"
# include "eyeline/utilx/Exception.hpp"

# include "eyeline/ss7na/common/Message.hpp"
# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/MessageHandlingDispatcherIface.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {
namespace messages {

enum smi_e { AffectedSubsystemMultiplicityUnknown = 0, Reserved_1, Reserved_2, Reserved_3 };

class SCMGMessage {
public:
  typedef uint8_t msg_code_t;

  SCMGMessage(msg_code_t msg_code)
  : _msgCode(msg_code), _isSetAffectedSSN(false), _isSetAffectedPC(false), _isSetSmi(false)
  {}

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual uint32_t getMsgCode() const { return _msgCode; }

  virtual const char* getMsgCodeTextDescription() const = 0;

  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() { return NULL; }

  void setAffectedSSN(uint8_t ssn) {
    _affectedSSN = ssn; _isSetAffectedSSN = true;
  }
  uint8_t getAffectedSSN() const {
    if ( _isSetAffectedSSN )
      return _affectedSSN;
    else
      throw utilx::FieldNotSetException("SCMGMessage::getAffectedSSN::: mandatory field isn't set");
  }

  void setAffectedPC(common::point_code_t pc) {
    if ( (pc & 0x3FFF) != pc )
      throw smsc::util::Exception("SCMGMessage::setAffectedPC::: invalid pc value = %04X: valid range is [0-2^14)",
                                  pc);
    _affectedPC = uint16_t(pc & 0xFF); _isSetAffectedPC = true;
  }
  common::point_code_t getAffectedPC() const {
    if ( _isSetAffectedPC )
      return _affectedPC;
    else
      throw utilx::FieldNotSetException("SCMGMessage::getAffectedPC::: mandatory field isn't set");
  }

  void setSMI(smi_e smi) {
    _smi = smi; _isSetSmi = true;
  }
  smi_e getSMI() const {
    if ( _isSetSmi )
      return _smi;
    else
      throw utilx::FieldNotSetException("SCMGMessage::getSMI::: mandatory field isn't set");
  }

  static const size_t UPPER_ESTIMATE_MAX_MESSAGE_SIZE = 32;

protected:
  size_t addMFixedField(common::TP* result_buf, size_t offset, uint8_t value) const;
  size_t addMFixedField(common::TP* result_buf, size_t offset, uint16_t value) const;
  size_t extractMFixedField(const common::TP& packet_buf, size_t offset,
                            uint8_t* value) const;
  size_t extractMFixedField(const common::TP& packet_buf, size_t offset,
                            uint16_t* value) const;

private:
  msg_code_t _msgCode;
  uint8_t _affectedSSN;
  uint16_t _affectedPC;
  smi_e _smi;
  bool _isSetAffectedSSN, _isSetAffectedPC, _isSetSmi;
};

}}}}}}

#include "eyeline/utilx/PreallocatedMemoryManager.hpp"
#include "eyeline/ss7na/common/types.hpp"

namespace eyeline {
namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_SCCP_SCMG_MESSAGE>() {
  static TSD_Init_Helper<MEM_FOR_SCCP_SCMG_MESSAGE> tsd_init;
  alloc_mem_desc_t* res = allocateMemory(ss7na::m3ua_gw::sccp::scmg::messages::SCMGMessage::UPPER_ESTIMATE_MAX_MESSAGE_SIZE,
                                         tsd_init._tsd_memory_key);

  return res;
}

}}

#endif
