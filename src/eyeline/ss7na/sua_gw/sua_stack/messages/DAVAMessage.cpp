#include <stdio.h>
#include "eyeline/utilx/Exception.hpp"

#include "DAVAMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const uint32_t DAVAMessage::_MSG_CODE;

size_t
DAVAMessage::serialize(common::TP* result_buf) const
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
DAVAMessage::deserialize(const common::TP& packet_buf)
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
DAVAMessage::toString() const
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
DAVAMessage::getLength() const
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
DAVAMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("DAVAMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

const TLV_SSN&
DAVAMessage::getSSN() const
{
  if ( _ssn.isSetValue() ) return _ssn;
  else throw utilx::FieldNotSetException("DAVAMessage::getSSN::: ssn wasn't set");
}

const TLV_SMI&
DAVAMessage::getSMI() const
{
  if ( _smi.isSetValue() ) return _smi;
  else throw utilx::FieldNotSetException("DAVAMessage::getSMI::: smi wasn't set");
}

const common::TLV_InfoString&
DAVAMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DAVAMessage::getInfoString::: infoString wasn't set");
}

}}}}}
