#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_XUDTS_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_XUDTS_HPP__

# include "eyeline/utilx/types.hpp"
# include "eyeline/utilx/Exception.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"

# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/Segmentation.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/ProtocolClass.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/SCCPMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/XUDT.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

class XUDTS : public SCCPMessage {
public:
  XUDTS()
  : SCCPMessage(_MSG_CODE), _returnCause(0), _hopCounter(0), _dataLen(0), _importance(0),
    _isSetReturnCause(false), _isSetHopCounter(false), _isSetCalledAddress(false),
    _isSetCallingAddress(false), _isSetData(false),_isSetSegmentation(false),
    _isSetImportance(false)
  {}

  XUDTS(const XUDT& xudt, common::return_cause_value_t cause);

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "XUDTS"; }

  void setReturnCause(common::return_cause_value_t return_cause) {
    _returnCause = return_cause; _isSetReturnCause = true;
  }
  uint8_t getReturnCause() const {
    if ( _isSetReturnCause )
      return _returnCause;
    else
      throw utilx::FieldNotSetException("XUDTS::getReturnCause::: mandatory field isn't set");
  }

  void setHopCounter(uint8_t hop_counter) {
    if ( !hop_counter || hop_counter > 15 )
      throw smsc::util::Exception("XUDT::setHopCounter::: invalid hop_counter value=%u, value should be in range [1-15]",
                                  hop_counter);
    _hopCounter = hop_counter; _isSetHopCounter = true;
  }
  uint8_t getHopCounter() const {
    if ( _isSetHopCounter )
      return _hopCounter;
    else
      throw utilx::FieldNotSetException("XUDT::getHopCounter::: mandatory field isn't set");
  }

  void setCalledAddress(const eyeline::sccp::SCCPAddress& called_address) {
    _calledAddress = called_address; _isSetCalledAddress = true;
  }
  const eyeline::sccp::SCCPAddress& getCalledAddress() const {
    if ( _isSetCalledAddress )
      return _calledAddress;
    else
      throw utilx::FieldNotSetException("XUDT::getCalledAddress::: mandatory field isn't set");
  }

  void setCallingAddress(const eyeline::sccp::SCCPAddress& calling_address) {
    _callingAddress = calling_address; _isSetCallingAddress = true;
  }
  const eyeline::sccp::SCCPAddress& getCallingAddress() const {
    if ( _isSetCallingAddress )
      return _callingAddress;
    else
      throw utilx::FieldNotSetException("XUDT::getCallingAddress::: mandatory field isn't set");
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
      throw utilx::FieldNotSetException("XUDT::getData::: mandatory field isn't set");
  }

  void setSegmentation(const Segmentation& segmentation) {
    _segmentation = segmentation; _isSetSegmentation = true;
  }
  const Segmentation& getSegmentation() const {
    if ( _isSetSegmentation )
      return _segmentation;
    else
      throw utilx::FieldNotSetException("XUDT::getSegmentation::: optional field isn't set");
  }
  bool isSetSegmentation() const { return _isSetSegmentation; }

  void setImportance(uint8_t importance) {
    if ( importance > 7 )
      throw smsc::util::Exception("XUDT::setImporance::: invalid importance value=%u, value should be in range [0-7]",
                                  importance);

    _importance = importance; _isSetImportance = true;
  }
  uint8_t getImportance() const {
    if ( _isSetImportance )
      return _importance;
    else
      throw utilx::FieldNotSetException("XUDT::getImportance::: optional field isn't set");
  }
  bool isSetImportance() const { return _isSetImportance; }

private:
  size_t parseMandatoryVariablePart(const common::TP& packet_buf, size_t offset,
                                    size_t* optional_part_ptr);
  size_t parseOptionalPart(const common::TP& packet_buf, size_t offset);

  static const msg_code_t _MSG_CODE = 0x12;
  uint8_t _returnCause;
  uint8_t _hopCounter;
  eyeline::sccp::SCCPAddress _calledAddress;
  eyeline::sccp::SCCPAddress _callingAddress;
  uint8_t _dataLen;
  uint8_t _data[255];
  Segmentation _segmentation;
  uint8_t _importance;
  bool _isSetReturnCause, _isSetHopCounter, _isSetCalledAddress, _isSetCallingAddress;
  bool _isSetData, _isSetSegmentation, _isSetImportance;
};

}}}}}

#endif
