#include "eyeline/utilx/Exception.hpp"

#include "DUNAMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const uint32_t DUNAMessage::_MSG_CODE;

size_t
DUNAMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(result_buf, offset);

  if ( _ssn.isSetValue() )
    offset = _ssn.serialize(result_buf, offset);

  if ( _smi.isSetValue() )
    offset = _smi.serialize(result_buf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);
  
  return offset;
}

size_t
DUNAMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_affectedPointCodes);
  tlvFactory.registerExpectedOptionalTlv(&_ssn);
  tlvFactory.registerExpectedOptionalTlv(&_smi);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
DUNAMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _affectedPointCodes.isSetValue() )
    result += "," + _affectedPointCodes.toString();

  if ( _ssn.isSetValue() )
    result += "," + _ssn.toString();

  if ( _smi.isSetValue() )
    result += "," + _smi.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
DUNAMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length = _routingContext.getLength();

  if ( _affectedPointCodes.isSetValue() )
    length += _affectedPointCodes.getLength();

  if ( _ssn.isSetValue() )
    length += _ssn.getLength();

  if ( _smi.isSetValue() )
    length += _smi.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const common::TLV_AffectedPointCode&
DUNAMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("DUNAMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

const TLV_SSN&
DUNAMessage::getSSN() const
{
  if ( _ssn.isSetValue() ) return _ssn;
  else throw utilx::FieldNotSetException("DUNAMessage::getSSN::: ssn wasn't set");
}

const TLV_SMI&
DUNAMessage::getSMI() const
{
  if ( _smi.isSetValue() ) return _smi;
  else throw utilx::FieldNotSetException("DUNAMessage::getSMI::: smi wasn't set");
}

const common::TLV_InfoString&
DUNAMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DUNAMessage::getInfoString::: infoString wasn't set");
}

}}}}}
