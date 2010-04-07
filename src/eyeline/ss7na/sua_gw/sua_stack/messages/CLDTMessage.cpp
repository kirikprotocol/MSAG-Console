#include <stdio.h>
#include "util/Exception.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "CLDTMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const uint32_t CLDTMessage::_MSG_CODE;

size_t
CLDTMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: routingContext is a mandatory field and wasn't set");

  if ( _protocolClass.isSetValue() )
    offset = _protocolClass.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: protocolClass is a mandatory field and wasn't set");

  if ( _sourceAddress.isSetValue() )
    offset = _sourceAddress.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: sourceAddress is a mandatory field and wasn't set");

  if ( _destinationAddress.isSetValue() )
    offset = _destinationAddress.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: destinationAddress is a mandatory field and wasn't set");

  if ( _sequenceControl.isSetValue() )
    offset = _sequenceControl.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: sequenceControl is a mandatory field and wasn't set");

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
    throw smsc::util::Exception("CLDT::serialize::: data is a mandatory field and wasn't set");

  result_buf->pkt_sctp_props.streamNo = getStreamNo();

  return offset;
}

size_t
CLDTMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedMandatoryTlv(&_routingContext);
  tlvFactory.registerExpectedMandatoryTlv(&_protocolClass);
  tlvFactory.registerExpectedMandatoryTlv(&_sourceAddress);
  tlvFactory.registerExpectedMandatoryTlv(&_destinationAddress);
  tlvFactory.registerExpectedMandatoryTlv(&_sequenceControl);

  tlvFactory.registerExpectedOptionalTlv(&_hopCount);
  tlvFactory.registerExpectedOptionalTlv(&_importance);
  tlvFactory.registerExpectedOptionalTlv(&_messagePriority);
  tlvFactory.registerExpectedOptionalTlv(&_correlationId);
  tlvFactory.registerExpectedOptionalTlv(&_segmentation);

  tlvFactory.registerExpectedMandatoryTlv(&_data);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
CLDTMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _protocolClass.isSetValue() )
    result += "," + _protocolClass.toString();

  if ( _sourceAddress.isSetValue() )
    result += "," + _sourceAddress.toString();

  if ( _destinationAddress.isSetValue() )
    result += "," + _destinationAddress.toString();

  if ( _sequenceControl.isSetValue() )
    result += "," + _sequenceControl.toString();

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
CLDTMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length += _routingContext.getLength();
  else
    throw smsc::util::Exception("CLDTMessage::getLength::: routingContext is a mandatory field and wasn't set");

  if ( _protocolClass.isSetValue() )
    length += _protocolClass.getLength();
  else
    throw smsc::util::Exception("CLDTMessage::getLength::: protocolClass is a mandatory field and wasn't set");

  if ( _sourceAddress.isSetValue() )
    length += _sourceAddress.getLength();
  else
    throw smsc::util::Exception("CLDTMessage::getLength::: sourceAddress is a mandatory field and wasn't set");

  if ( _destinationAddress.isSetValue() )
    length += _destinationAddress.getLength();
  else
    throw smsc::util::Exception("CLDTMessage::getLength::: destinationAddress is a mandatory field and wasn't set");

  if ( _sequenceControl.isSetValue() )
    length += _sequenceControl.getLength();
  else
    throw smsc::util::Exception("CLDTMessage::getLength::: sequenceControl is a mandatory field and wasn't set");

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
    throw smsc::util::Exception("CLDTMessage::getLength::: data is a mandatory field and wasn't set");

  return HEADER_SIZE + length;
}

const TLV_ProtocolClass&
CLDTMessage::getProtocolClass() const
{
  if ( _protocolClass.isSetValue() ) return _protocolClass;
  else throw utilx::FieldNotSetException("CLDTMessage::getProtocolClass::: protocolClass wasn't set");
}

const TLV_SourceAddress&
CLDTMessage::getSourceAddress() const
{
  if ( _sourceAddress.isSetValue() ) return _sourceAddress;
  else throw utilx::FieldNotSetException("CLDTMessage::getSourceAddress::: sourceAddress wasn't set");
}

const TLV_DestinationAddress&
CLDTMessage::getDestinationAddress() const
{
  if ( _destinationAddress.isSetValue() ) return _destinationAddress;
  else throw utilx::FieldNotSetException("CLDTMessage::getDestinationAddress::: destinationAddress wasn't set");
}

const TLV_SequenceControl&
CLDTMessage::getSequenceControl() const
{
  if ( _sequenceControl.isSetValue() ) return _sequenceControl;
  else throw utilx::FieldNotSetException("CLDTMessage::getSequenceControl::: sequenceControl wasn't set");
}

const TLV_SS7HopCount&
CLDTMessage::getSS7HopCount() const
{
  if ( _hopCount.isSetValue() ) return _hopCount;
  else throw utilx::FieldNotSetException("CLDTMessage::getSS7HopCount::: hopCount wasn't set");
}

const TLV_Importance&
CLDTMessage::getImportance() const
{
  if ( _importance.isSetValue() ) return _importance;
  else throw utilx::FieldNotSetException("CLDTMessage::getImportance::: importance wasn't set");
}

const TLV_MessagePriority&
CLDTMessage::getMessagePriority() const
{
  if ( _messagePriority.isSetValue() ) return _messagePriority;
  else throw utilx::FieldNotSetException("CLDTMessage::getMessagePriority::: messagePriority wasn't set");
}

const common::TLV_CorrelationId&
CLDTMessage::getCorrelationId() const
{
  if ( _correlationId.isSetValue() ) return _correlationId;
  else throw utilx::FieldNotSetException("CLDTMessage::getCorrelationId::: correlationId wasn't set");
}

const TLV_Segmentation&
CLDTMessage::getSegmentation() const
{
  if ( _segmentation.isSetValue() ) return _segmentation;
  else throw utilx::FieldNotSetException("CLDTMessage::getSegmentation::: segmentation wasn't set");
}

const TLV_Data&
CLDTMessage::getData() const
{
  if ( _data.isSetValue() ) return _data;
  else throw utilx::FieldNotSetException("CLDTMessage::getData::: data wasn't set");
}

}}}}}
