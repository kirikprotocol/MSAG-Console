#include "DATAMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

size_t
DATAMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _networkAppearance.isSetValue() )
    offset = _networkAppearance.serialize(result_buf, offset);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);

  if ( _protocolData.isSetValue() )
    offset = _protocolData.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("DATAMessage::serialize::: protocolData is a mandatory field and wasn't set");

  if ( _correlationId.isSetValue() )
    offset = _correlationId.serialize(result_buf, offset);

  return offset;
}

size_t
DATAMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  M3uaTLVFactory tlvFactory;
  tlvFactory.registerExpectedMandatoryTlv(&_protocolData);

  tlvFactory.registerExpectedOptionalTlv(&_networkAppearance);
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_correlationId);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
DATAMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _networkAppearance.isSetValue() )
    result += "," + _networkAppearance.toString();

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _protocolData.isSetValue() )
    result += "," + _protocolData.toString();

  if ( _correlationId.isSetValue() )
    result += "," + _correlationId.toString();

  return result;
}

const TLV_NetworkAppearance&
DATAMessage::getNetworkAppearance() const
{
  if ( _networkAppearance.isSetValue() ) return _networkAppearance;
  else throw utilx::FieldNotSetException("DATAMessage::getNetworkAppearance::: networkAppearance wasn't set");
}

const common::TLV_RoutingContext&
DATAMessage::getRoutingContext() const
{
  if ( _routingContext.isSetValue() ) return _routingContext;
  else throw utilx::FieldNotSetException("DATAMessage::getRoutingContext::: routingContext wasn't set");
}

const TLV_ProtocolData&
DATAMessage::getProtocolData() const
{
  if ( _protocolData.isSetValue() ) return _protocolData;
  else throw utilx::FieldNotSetException("DATAMessage::getProtocolData::: protocolData wasn't set");
}

const common::TLV_CorrelationId&
DATAMessage::getCorrelationId() const
{
  if ( _correlationId.isSetValue() ) return _correlationId;
  else throw utilx::FieldNotSetException("DATAMessage::getCorrelationId::: correlationId wasn't set");
}

uint32_t
DATAMessage::getLength() const
{
  uint32_t length=0;

  if ( _networkAppearance.isSetValue() )
    length = _networkAppearance.getLength();

  if ( _routingContext.isSetValue() )
    length += _routingContext.getLength();

  if ( _protocolData.isSetValue() )
    length += _protocolData.getLength();

  if ( _correlationId.isSetValue() )
    length += _correlationId.getLength();

  return HEADER_SIZE + length;
}

}}}}}}
