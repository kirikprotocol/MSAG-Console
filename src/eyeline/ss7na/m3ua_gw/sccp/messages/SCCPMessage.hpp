#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_SCCPMESSAGE_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_SCCPMESSAGE_HPP__

# include <sys/types.h>

# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/Message.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/MessageHandlingDispatcherIface.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

class SCCPMessage {
public:
  typedef uint8_t msg_code_t;

  SCCPMessage(msg_code_t msg_code)
  : _msgCode(msg_code), _sls(0), _isSetSls(false) {}

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const = 0;

  virtual uint32_t getMsgCode() const { return _msgCode; }

  virtual const char* getMsgCodeTextDescription() const = 0;

  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() { return NULL; }

  void setSLS(uint8_t value) { _sls = value; _isSetSls = true; }
  uint8_t getSLS() const {
    if ( _isSetSls )
      return _sls;
    else
      throw utilx::FieldNotSetException("SCCPMessage::getSLS:: value isn't set");
  }

protected:
  size_t addMFixedField(common::TP* result_buf, size_t offset, uint8_t value) const;
  size_t addOneOctetPointer(common::TP* result_buf, size_t offset) const;
  size_t addMVariablePartValue(common::TP* result_buf, size_t offset, size_t pointer_to_param,
                               unsigned data_len, const uint8_t* data) const;
  size_t addOptionalParameter(common::TP* result_buf, size_t offset,
                              uint8_t tag, uint8_t value_length, const uint8_t* value) const;
  size_t addEndOfOptionalParams(common::TP* result_buf, size_t offset) const;
  void updateOptinalPointer(common::TP* result_buf, size_t optional_part_ptr,
                            size_t begin_optional_part) const;

  size_t extractMFixedField(const common::TP& packet_buf, size_t offset, uint8_t* value) const;
  size_t extractOneOctetPointer(const common::TP& packet_buf, size_t offset, uint8_t* value) const;
  size_t extractParamName(const common::TP& packet_buf, size_t offset, uint8_t* value) const;
  size_t extractLengthIndicator(const common::TP& packet_buf, size_t offset, uint8_t* value) const;

private:
  msg_code_t _msgCode;
  uint8_t _sls;
  bool _isSetSls;
  // disable object creation in heap
  static void operator delete (void*) {}
};

}}}}}

#endif
