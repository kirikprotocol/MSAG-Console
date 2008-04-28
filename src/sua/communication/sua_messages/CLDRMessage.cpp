#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "CLDRMessage.hpp"
#include <logger/Logger.h>

namespace sua_messages {

const uint32_t CLDRMessage::_MSG_CODE;

CLDRMessage::CLDRMessage ()
  : SUAMessage(_MSG_CODE)
{
  setStreamNo(1);
}

CLDRMessage::CLDRMessage (const CLDTMessage& cldtMessage, communication::return_cause_value_t causeValue)
  : SUAMessage(_MSG_CODE)
{
  setRoutingContext(cldtMessage.getRoutingContext());
  setSourceAddress(TLV_SourceAddress(cldtMessage.getDestinationAddress().getGlobalTitle()));
  setDestinationAddress(TLV_DestinationAddress(cldtMessage.getSourceAddress().getGlobalTitle()));
  setSCCPCause(TLV_SCCP_Cause(communication::RETURN_CAUSE, causeValue));

  try {
    setImportance(cldtMessage.getImportance());
  } catch (utilx::FieldNotSetException& ex) {}

  try {
    setMessagePriority(cldtMessage.getMessagePriority());
  } catch (utilx::FieldNotSetException& ex) {}

  try {
    setCorrelationId(cldtMessage.getCorrelationId());
  } catch (utilx::FieldNotSetException& ex) {}

  try {
    setSegmentation(cldtMessage.getSegmentation());
  } catch (utilx::FieldNotSetException& ex) {}

  setData(cldtMessage.getData());

  setStreamNo(1);
}

size_t
CLDRMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("CLDRMessage::serialize::: routingContext is a mandatory field and wasn't set");

  if ( _sccpCause.isSetValue() )
    offset = _sccpCause.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("CLDRMessage::serialize::: protocolClass is a mandatory field and wasn't set");

  if ( _sourceAddress.isSetValue() )
    offset = _sourceAddress.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("CLDRMessage::serialize::: sourceAddress is a mandatory field and wasn't set");

  if ( _destinationAddress.isSetValue() )
    offset = _destinationAddress.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("CLDRMessage::serialize::: destinationAddress is a mandatory field and wasn't set");

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
    throw smsc::util::Exception("CLDRMessage::serialize::: data is a mandatory field and wasn't set");

  resultBuf->pkt_sctp_props.streamNo = getStreamNo();

  return offset;
}

size_t
CLDRMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

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

  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
CLDRMessage::toString() const
{
  std::string result(SUAMessage::toString());

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

const char*
CLDRMessage::getMsgCodeTextDescription() const
{
  return "CLDR";
}

void
CLDRMessage::setSCCPCause(const TLV_SCCP_Cause& sccpCause)
{
  _sccpCause = sccpCause;
}

const TLV_SCCP_Cause&
CLDRMessage::getSCCPCause() const
{
  if ( _sccpCause.isSetValue() ) return _sccpCause;
  else throw utilx::FieldNotSetException("CLDRMessage::getSCCPCause::: sccpCause wasn't set");
}

void
CLDRMessage::setSourceAddress(const TLV_SourceAddress& sourceAddress)
{
  _sourceAddress = sourceAddress;
}

const TLV_SourceAddress&
CLDRMessage::getSourceAddress() const
{
  if ( _sourceAddress.isSetValue() ) return _sourceAddress;
  else throw utilx::FieldNotSetException("CLDRMessage::getSourceAddress::: sourceAddress wasn't set");
}

void
CLDRMessage::setDestinationAddress(const TLV_DestinationAddress& destinationAddress)
{
  _destinationAddress = destinationAddress;
}

const TLV_DestinationAddress&
CLDRMessage::getDestinationAddress() const
{
  if ( _destinationAddress.isSetValue() ) return _destinationAddress;
  else throw utilx::FieldNotSetException("CLDRMessage::getDestinationAddress::: destinationAddress wasn't set");
}

void
CLDRMessage::setSS7HopCount(const TLV_SS7HopCount& ss7HopCount)
{
  _hopCount = ss7HopCount;
}

const TLV_SS7HopCount&
CLDRMessage::getSS7HopCount() const
{
  if ( _hopCount.isSetValue() ) return _hopCount;
  else throw utilx::FieldNotSetException("CLDRMessage::getSS7HopCount::: hopCount wasn't set");
}

void
CLDRMessage::setImportance(const TLV_Importance& importance)
{
  _importance = importance;
}

const TLV_Importance&
CLDRMessage::getImportance() const
{
  if ( _importance.isSetValue() ) return _importance;
  else throw utilx::FieldNotSetException("CLDRMessage::getImportance::: importance wasn't set");
}

void
CLDRMessage::setMessagePriority(const TLV_MessagePriority& messagePriority)
{
  _messagePriority = messagePriority;
}

const TLV_MessagePriority&
CLDRMessage::getMessagePriority() const
{
  if ( _messagePriority.isSetValue() ) return _messagePriority;
  else throw utilx::FieldNotSetException("CLDRMessage::getMessagePriority::: messagePriority wasn't set");
}

void
CLDRMessage::setCorrelationId(const TLV_CorrelationId& correlationId)
{
  _correlationId = correlationId;
}

const TLV_CorrelationId&
CLDRMessage::getCorrelationId() const
{
  if ( _correlationId.isSetValue() ) return _correlationId;
  else throw utilx::FieldNotSetException("CLDRMessage::getCorrelationId::: correlationId wasn't set");
}

void
CLDRMessage::setSegmentation(const TLV_Segmentation& segmentation)
{
  _segmentation = segmentation;
}

const TLV_Segmentation&
CLDRMessage::getSegmentation() const
{
  if ( _segmentation.isSetValue() ) return _segmentation;
  else throw utilx::FieldNotSetException("CLDRMessage::getSegmentation::: segmentation wasn't set");
}

void
CLDRMessage::setData(const TLV_Data& data)
{
  _data = data;
}

const TLV_Data&
CLDRMessage::getData() const
{
  if ( _data.isSetValue() ) return _data;
  else throw utilx::FieldNotSetException("CLDRMessage::getData::: data wasn't set");
}

}
