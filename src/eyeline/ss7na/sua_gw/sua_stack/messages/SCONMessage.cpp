#include "util/Exception.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "SCONMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const uint32_t SCONMessage::_MSG_CODE;

size_t
SCONMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(result_buf, offset);

  if ( _ssn.isSetValue() )
    offset = _ssn.serialize(result_buf, offset);

  if ( _congestionLevel.isSetValue() )
    offset = _congestionLevel.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("SCONMessage::getLength::: congestionLevel is a mandatory field and wasn't set");

  if ( _smi.isSetValue() )
    offset = _smi.serialize(result_buf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);
  
  return offset;
}

size_t
SCONMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_affectedPointCodes);
  tlvFactory.registerExpectedOptionalTlv(&_ssn);
  tlvFactory.registerExpectedMandatoryTlv(&_congestionLevel);
  tlvFactory.registerExpectedOptionalTlv(&_smi);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
SCONMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _affectedPointCodes.isSetValue() )
    result += "," + _affectedPointCodes.toString();

  if ( _ssn.isSetValue() )
    result += "," + _ssn.toString();

  if ( _congestionLevel.isSetValue() )
    result += "," + _congestionLevel.toString();

  if ( _smi.isSetValue() )
    result += "," + _smi.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
SCONMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length = _routingContext.getLength();

  if ( _affectedPointCodes.isSetValue() )
    length += _affectedPointCodes.getLength();

  if ( _ssn.isSetValue() )
    length += _ssn.getLength();

  if ( _congestionLevel.isSetValue() )
    length += _congestionLevel.getLength();
  else
    throw smsc::util::Exception("SCONMessage::getLength::: congestionLevel is a mandatory field and wasn't set");

  if ( _smi.isSetValue() )
    length += _smi.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const common::TLV_AffectedPointCode&
SCONMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("SCONMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

const TLV_SSN&
SCONMessage::getSSN() const
{
  if ( _ssn.isSetValue() ) return _ssn;
  else throw utilx::FieldNotSetException("SCONMessage::getSSN::: ssn wasn't set");
}

const TLV_CongestionLevel&
SCONMessage::getCongestionLevel() const
{
  if ( _congestionLevel.isSetValue() ) return _congestionLevel;
  else throw utilx::FieldNotSetException("SCONMessage::getCongestionLevelI::: congestionLevel wasn't set");
}

const TLV_SMI&
SCONMessage::getSMI() const
{
  if ( _smi.isSetValue() ) return _smi;
  else throw utilx::FieldNotSetException("SCONMessage::getSMI::: smi wasn't set");
}

const common::TLV_InfoString&
SCONMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("SCONMessage::getInfoString::: infoString wasn't set");
}

}}}}}
