#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_UDT_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_UDT_HPP__

# include "eyeline/utilx/types.hpp"
# include "eyeline/utilx/Exception.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"

# include "eyeline/ss7na/m3ua_gw/sccp/messages/ProtocolClass.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/SCCPMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

class UDT : public SCCPMessage {
public:
  UDT()
  : SCCPMessage(_MSG_CODE), _protocolClass(0),
    _dataLen(0), _isSetProtocolClass(false), _isSetCalledAddress(false),
    _isSetCallingAddress(false), _isSetData(false)
  {}

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "UDT"; }

  void setProtocolClass(const ProtocolClass& protocol_class) {
    _protocolClass = protocol_class; _isSetProtocolClass = true;
  }
  const ProtocolClass& getProtocolClass() const {
    if ( _isSetProtocolClass )
      return _protocolClass;
    else
      throw utilx::FieldNotSetException("UDT::getProtocolClass::: mandatory field isn't set");
  }

  void setCalledAddress(const eyeline::sccp::SCCPAddress& called_address) {
    _calledAddress = called_address; _isSetCalledAddress = true;
  }
  const eyeline::sccp::SCCPAddress& getCalledAddress() const {
    if ( _isSetCalledAddress )
      return _calledAddress;
    else
      throw utilx::FieldNotSetException("UDT::getCalledAddress::: mandatory field isn't set");
  }

  void setCallingAddress(const eyeline::sccp::SCCPAddress& calling_address) {
    _callingAddress = calling_address; _isSetCallingAddress = true;
  }
  const eyeline::sccp::SCCPAddress& getCallingAddress() const {
    if ( _isSetCallingAddress )
      return _callingAddress;
    else
      throw utilx::FieldNotSetException("UDT::getCallingAddress::: mandatory field isn't set");
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
      throw utilx::FieldNotSetException("UDT::getData::: mandatory field isn't set");
  }

private:
  static const msg_code_t _MSG_CODE = 0x09;
  ProtocolClass _protocolClass;
  eyeline::sccp::SCCPAddress _calledAddress;
  eyeline::sccp::SCCPAddress _callingAddress;
  uint8_t _dataLen;
  uint8_t _data[255];
  bool _isSetProtocolClass, _isSetCalledAddress, _isSetCallingAddress, _isSetData;
};

}}}}}

#endif
