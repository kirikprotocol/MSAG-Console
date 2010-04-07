#include <stdio.h>
#include "util/Exception.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "CLDRMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const uint32_t CLDRMessage::_MSG_CODE;

CLDRMessage::CLDRMessage(const CLDTMessage& original_message,
                         common::return_cause_value_t cause_value)
  : common::AdaptationLayer_Message(_MSG_CODE)
{
  setRoutingContext(original_message.getRoutingContext());
  setSourceAddress(TLV_SourceAddress(original_message.getDestinationAddress().getGlobalTitle()));
  setDestinationAddress(TLV_DestinationAddress(original_message.getSourceAddress().getGlobalTitle()));
  setSCCPCause(TLV_SCCP_Cause(common::RETURN_CAUSE, cause_value));

  try {
    setImportance(original_message.getImportance());
  } catch (utilx::FieldNotSetException& ex) {}

  try {
    setMessagePriority(original_message.getMessagePriority());
  } catch (utilx::FieldNotSetException& ex) {}

  try {
    setCorrelationId(original_message.getCorrelationId());
  } catch (utilx::FieldNotSetException& ex) {}

  try {
    setSegmentation(original_message.getSegmentation());
  } catch (utilx::FieldNotSetException& ex) {}

  setData(original_message.getData());

  setStreamNo(1);
}

size_t
CLDRMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("CLDRMessage::serialize::: routingContext is a mandatory field and wasn't set");

  if ( _sccpCause.isSetValue() )
    offset = _sccpCause.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("CLDRMessage::serialize::: protocolClass is a mandatory field and wasn't set");

  if ( _sourceAddress.isSetValue() )
    offset = _sourceAddress.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("CLDRMessage::serialize::: sourceAddress is a mandatory field and wasn't set");

  if ( _destinationAddress.isSetValue() )
    offset = _destinationAddress.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("CLDRMessage::serialize::: destinationAddress is a mandatory field and wasn't set");

  if ( _hopCount.isSetValue() )
    offset = _hopCount.serialize(result_buf, offset);

  if ( _importance.isSetValue() )
    offset = _importance.serialize(result_buf, offset);

  if ( _messagePriority.isSetValue() )
    offset = _messagePriority.serialize(result_buf, offset);

  if ( _correlationId.isSetValue() )
    offset = _correlationId.serialize(result_buf, offset);

  if ( _segmentation.isSetValue() )
    offset = _segmentation.serialize(result_buf, offset);

  if ( _data.isSetValue() )
    offset = _data.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("CLDRMessage::serialize::: data is a mandatory field and wasn't set");

  result_buf->pkt_sctp_props.streamNo = getStreamNo();

  return offset;
}

size_t
CLDRMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedMandatoryTlv(&_routingContext);
  tlvFactory.registerExpectedMandatoryTlv(&_sccpCause);
  tlvFactory.registerExpectedMandatoryTlv(&_sourceAddress);
  tlvFactory.registerExpectedMandatoryTlv(&_destinationAddress);

  tlvFactory.registerExpectedOptionalTlv(&_hopCount);
  tlvFactory.registerExpectedOptionalTlv(&_importance);
  tlvFactory.registerExpectedOptionalTlv(&_messagePriority);
  tlvFactory.registerExpectedOptionalTlv(&_correlationId);
  tlvFactory.registerExpectedOptionalTlv(&_segmentation);

  tlvFactory.registerExpectedMandatoryTlv(&_data);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
CLDRMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _sccpCause.isSetValue() )
    result += "," + _sccpCause.toString();

  if ( _sourceAddress.isSetValue() )
    result += "," + _sourceAddress.toString();

  if ( _destinationAddress.isSetValue() )
    result += "," + _destinationAddress.toString();

  if ( _hopCount.isSetValue() )
    result += "," + _hopCount.toString();

  if ( _importance.isSetValue() )
    result += "," + _importance.toString();

  if ( _messagePriority.isSetValue() )
    result += "," + _messagePriority.toString();

  if ( _correlationId.isSetValue() )
    result += "," + _correlationId.toString();

  if ( _segmentation.isSetValue() )
    result += "," + _segmentation.toString();

  if ( _data.isSetValue() )
    result += "," + _data.toString();

  return result;
}

uint32_t
CLDRMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length += _routingContext.getLength();
  else
    throw smsc::util::Exception("CLDRMessage::getLength::: routingContext is a mandatory field and wasn't set");

  if ( _sccpCause.isSetValue() )
    length += _sccpCause.getLength();
  else
    throw smsc::util::Exception("CLDRMessage::getLength::: sccpCause is a mandatory field and wasn't set");

  if ( _sourceAddress.isSetValue() )
    length += _sourceAddress.getLength();
  else
    throw smsc::util::Exception("CLDRMessage::getLength::: sourceAddress is a mandatory field and wasn't set");

  if ( _destinationAddress.isSetValue() )
    length += _destinationAddress.getLength();
  else
    throw smsc::util::Exception("CLDRMessage::getLength::: destinationAddress is a mandatory field and wasn't set");

  if ( _hopCount.isSetValue() )
    length += _hopCount.getLength();

  if ( _importance.isSetValue() )
    length += _importance.getLength();

  if ( _messagePriority.isSetValue() )
    length += _messagePriority.getLength();

  if ( _correlationId.isSetValue() )
    length += _correlationId.getLength();

  if ( _segmentation.isSetValue() )
    length += _segmentation.getLength();

  if ( _data.isSetValue() )
    length += _data.getLength();
  else
    throw smsc::util::Exception("CLDRMessage::getLength::: data is a mandatory field and wasn't set");

  return HEADER_SIZE + length;
}

const TLV_SCCP_Cause&
CLDRMessage::getSCCPCause() const
{
  if ( _sccpCause.isSetValue() ) return _sccpCause;
  else throw utilx::FieldNotSetException("CLDRMessage::getSCCPCause::: sccpCause wasn't set");
}

const TLV_SourceAddress&
CLDRMessage::getSourceAddress() const
{
  if ( _sourceAddress.isSetValue() ) return _sourceAddress;
  else throw utilx::FieldNotSetException("CLDRMessage::getSourceAddress::: sourceAddress wasn't set");
}

const TLV_DestinationAddress&
CLDRMessage::getDestinationAddress() const
{
  if ( _destinationAddress.isSetValue() ) return _destinationAddress;
  else throw utilx::FieldNotSetException("CLDRMessage::getDestinationAddress::: destinationAddress wasn't set");
}

const TLV_SS7HopCount&
CLDRMessage::getSS7HopCount() const
{
  if ( _hopCount.isSetValue() ) return _hopCount;
  else throw utilx::FieldNotSetException("CLDRMessage::getSS7HopCount::: hopCount wasn't set");
}

const TLV_Importance&
CLDRMessage::getImportance() const
{
  if ( _importance.isSetValue() ) return _importance;
  else throw utilx::FieldNotSetException("CLDRMessage::getImportance::: importance wasn't set");
}

const TLV_MessagePriority&
CLDRMessage::getMessagePriority() const
{
  if ( _messagePriority.isSetValue() ) return _messagePriority;
  else throw utilx::FieldNotSetException("CLDRMessage::getMessagePriority::: messagePriority wasn't set");
}

const common::TLV_CorrelationId&
CLDRMessage::getCorrelationId() const
{
  if ( _correlationId.isSetValue() ) return _correlationId;
  else throw utilx::FieldNotSetException("CLDRMessage::getCorrelationId::: correlationId wasn't set");
}

const TLV_Segmentation&
CLDRMessage::getSegmentation() const
{
  if ( _segmentation.isSetValue() ) return _segmentation;
  else throw utilx::FieldNotSetException("CLDRMessage::getSegmentation::: segmentation wasn't set");
}

const TLV_Data&
CLDRMessage::getData() const
{
  if ( _data.isSetValue() ) return _data;
  else throw utilx::FieldNotSetException("CLDRMessage::getData::: data wasn't set");
}

}}}}}
