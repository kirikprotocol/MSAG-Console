#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_UDTS_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_UDTS_HPP__

# include "eyeline/utilx/types.hpp"
# include "eyeline/utilx/Exception.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"

# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/UDT.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/ProtocolClass.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/SCCPMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

class UDTS : public SCCPMessage {
public:
  UDTS()
  : SCCPMessage(_MSG_CODE), _returnCause(0),
    _dataLen(0), _isSetReturnCause(false), _isSetCalledAddress(false),
    _isSetCallingAddress(false), _isSetData(false)
  {}

  UDTS(const UDT& udt, common::return_cause_value_t cause);

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "UDTS"; }

  void setReturnCause(common::return_cause_value_t return_cause) {
    _returnCause = return_cause; _isSetReturnCause = true;
  }
  uint8_t getReturnCause() const {
    if ( _isSetReturnCause )
      return _returnCause;
    else
      throw utilx::FieldNotSetException("UDTS::getReturnCause::: mandatory field isn't set");
  }

  void setCalledAddress(const eyeline::sccp::SCCPAddress& called_address) {
    _calledAddress = called_address; _isSetCalledAddress = true;
  }
  const eyeline::sccp::SCCPAddress& getCalledAddress() const {
    if ( _isSetCalledAddress )
      return _calledAddress;
    else
      throw utilx::FieldNotSetException("UDTS::getCalledAddress::: mandatory field isn't set");
  }

  void setCallingAddress(const eyeline::sccp::SCCPAddress& calling_address) {
    _callingAddress = calling_address; _isSetCallingAddress = true;
  }
  const eyeline::sccp::SCCPAddress& getCallingAddress() const {
    if ( _isSetCallingAddress )
      return _callingAddress;
    else
      throw utilx::FieldNotSetException("UDTS::getCallingAddress::: mandatory field isn't set");
  }

  void setData(const uint8_t* data, uint8_t data_len) {
    _dataLen = data_len;
    memcpy(_data, data, _dataLen);
    _isSetData = true;
  }
  utilx::variable_data_t getData() const {
    if ( _isSetData )
      return utilx::variable_data_t(_data, _dataLen);
    else
      throw utilx::FieldNotSetException("UDTS::getData::: mandatory field isn't set");
  }

private:
  static const msg_code_t _MSG_CODE = 0x0A;
  uint8_t _returnCause;
  eyeline::sccp::SCCPAddress _calledAddress;
  eyeline::sccp::SCCPAddress _callingAddress;
  uint8_t _dataLen;
  uint8_t _data[255];
  bool _isSetReturnCause, _isSetCalledAddress, _isSetCallingAddress, _isSetData;
};

}}}}}

#endif
