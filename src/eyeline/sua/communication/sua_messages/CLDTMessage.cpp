#include <stdio.h>
#include <util/Exception.hpp>
#include <eyeline/utilx/Exception.hpp>

#include "CLDTMessage.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

const uint32_t CLDTMessage::_MSG_CODE;

CLDTMessage::CLDTMessage ()
  : SUAMessage(_MSG_CODE)
{
  setStreamNo(1);
}

size_t
CLDTMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: routingContext is a mandatory field and wasn't set");

  if ( _protocolClass.isSetValue() )
    offset = _protocolClass.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: protocolClass is a mandatory field and wasn't set");

  if ( _sourceAddress.isSetValue() )
    offset = _sourceAddress.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: sourceAddress is a mandatory field and wasn't set");

  if ( _destinationAddress.isSetValue() )
    offset = _destinationAddress.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: destinationAddress is a mandatory field and wasn't set");

  if ( _sequenceControl.isSetValue() )
    offset = _sequenceControl.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: sequenceControl is a mandatory field and wasn't set");

  if ( _hopCount.isSetValue() )
    offset = _hopCount.serialize(resultBuf, offset);

  if ( _importance.isSetValue() )
    offset = _importance.serialize(resultBuf, offset);

  if ( _messagePriority.isSetValue() )
    offset = _messagePriority.serialize(resultBuf, offset);

  if ( _correlationId.isSetValue() )
    offset = _correlationId.serialize(resultBuf, offset);

  if ( _segmentation.isSetValue() )
    offset = _segmentation.serialize(resultBuf, offset);

  if ( _data.isSetValue() )
    offset = _data.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("CLDT::serialize::: data is a mandatory field and wasn't set");

  resultBuf->pkt_sctp_props.streamNo = getStreamNo();

  return offset;
}

size_t
CLDTMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

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

  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
CLDTMessage::toString() const
{
  std::string result(SUAMessage::toString());

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

const char*
CLDTMessage::getMsgCodeTextDescription() const
{
  return "CLDT";
}

void
CLDTMessage::setProtocolClass(const TLV_ProtocolClass& protocolClass)
{
  _protocolClass = protocolClass;
}

const TLV_ProtocolClass&
CLDTMessage::getProtocolClass() const
{
  if ( _protocolClass.isSetValue() ) return _protocolClass;
  else throw utilx::FieldNotSetException("CLDTMessage::getProtocolClass::: protocolClass wasn't set");
}

void
CLDTMessage::setSourceAddress(const TLV_SourceAddress& sourceAddress)
{
  _sourceAddress = sourceAddress;
}

const TLV_SourceAddress&
CLDTMessage::getSourceAddress() const
{
  if ( _sourceAddress.isSetValue() ) return _sourceAddress;
  else throw utilx::FieldNotSetException("CLDTMessage::getSourceAddress::: sourceAddress wasn't set");
}

void
CLDTMessage::setDestinationAddress(const TLV_DestinationAddress& destinationAddress)
{
  _destinationAddress = destinationAddress;
}

const TLV_DestinationAddress&
CLDTMessage::getDestinationAddress() const
{
  if ( _destinationAddress.isSetValue() ) return _destinationAddress;
  else throw utilx::FieldNotSetException("CLDTMessage::getDestinationAddress::: destinationAddress wasn't set");
}

void
CLDTMessage::setSequenceControl(const TLV_SequenceControl& sequenceControl)
{
  _sequenceControl = sequenceControl;
}

const TLV_SequenceControl&
CLDTMessage::getSequenceControl() const
{
  if ( _sequenceControl.isSetValue() ) return _sequenceControl;
  else throw utilx::FieldNotSetException("CLDTMessage::getSequenceControl::: sequenceControl wasn't set");
}

void
CLDTMessage::setSS7HopCount(const TLV_SS7HopCount& ss7HopCount)
{
  _hopCount = ss7HopCount;
}

const TLV_SS7HopCount&
CLDTMessage::getSS7HopCount() const
{
  if ( _hopCount.isSetValue() ) return _hopCount;
  else throw utilx::FieldNotSetException("CLDTMessage::getSS7HopCount::: hopCount wasn't set");
}

void
CLDTMessage::setImportance(const TLV_Importance& importance)
{
  _importance = importance;
}

const TLV_Importance&
CLDTMessage::getImportance() const
{
  if ( _importance.isSetValue() ) return _importance;
  else throw utilx::FieldNotSetException("CLDTMessage::getImportance::: importance wasn't set");
}

void
CLDTMessage::setMessagePriority(const TLV_MessagePriority& messagePriority)
{
  _messagePriority = messagePriority;
}

const TLV_MessagePriority&
CLDTMessage::getMessagePriority() const
{
  if ( _messagePriority.isSetValue() ) return _messagePriority;
  else throw utilx::FieldNotSetException("CLDTMessage::getMessagePriority::: messagePriority wasn't set");
}

void
CLDTMessage::setCorrelationId(const TLV_CorrelationId& correlationId)
{
  _correlationId = correlationId;
}

const TLV_CorrelationId&
CLDTMessage::getCorrelationId() const
{
  if ( _correlationId.isSetValue() ) return _correlationId;
  else throw utilx::FieldNotSetException("CLDTMessage::getCorrelationId::: correlationId wasn't set");
}

void
CLDTMessage::setSegmentation(const TLV_Segmentation& segmentation)
{
  _segmentation = segmentation;
}

const TLV_Segmentation&
CLDTMessage::getSegmentation() const
{
  if ( _segmentation.isSetValue() ) return _segmentation;
  else throw utilx::FieldNotSetException("CLDTMessage::getSegmentation::: segmentation wasn't set");
}

void
CLDTMessage::setData(const TLV_Data& data)
{
  _data = data;
}

const TLV_Data&
CLDTMessage::getData() const
{
  if ( _data.isSetValue() ) return _data;
  else throw utilx::FieldNotSetException("CLDTMessage::getData::: data wasn't set");
}

}}}}
