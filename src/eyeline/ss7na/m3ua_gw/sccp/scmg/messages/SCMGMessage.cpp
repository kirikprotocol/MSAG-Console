#include "SCMGMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {
namespace messages {

size_t
SCMGMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::addField(result_buf, 0, _msgCode);

  if ( !_isSetAffectedSSN )
    throw utilx::SerializationException("SCMGMessage::serialize::: mandatory field affectedSSN isn't set");
  offset = addMFixedField(result_buf, offset, _affectedSSN);

  if ( !_isSetAffectedPC )
    throw utilx::SerializationException("SCMGMessage::serialize::: mandatory field affectedPC isn't set");
  offset = addMFixedField(result_buf, offset, _affectedPC);

  if ( !_isSetSmi )
    throw utilx::SerializationException("SCMGMessage::serialize::: mandatory field smi isn't set");
  offset = addMFixedField(result_buf, offset, uint8_t(_smi));

  return offset;
}

size_t
SCMGMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::extractField(packet_buf, 0, &_msgCode);

  offset = extractMFixedField(packet_buf, offset, &_affectedSSN);
  _isSetAffectedSSN = true;
  offset = extractMFixedField(packet_buf, offset, &_affectedPC);
  _isSetAffectedPC = true;
  uint8_t oneOctetValue;
  offset = extractMFixedField(packet_buf, offset, &oneOctetValue);
  if ( oneOctetValue > Reserved_3 )
    throw utilx::DeserializationException("SCMGMessage::deserialize::: invalid smi value=%d, valid values range is [0-3]",
                                          oneOctetValue);
  _smi = smi_e(oneOctetValue); _isSetSmi = true;

  return offset;
}

std::string
SCMGMessage::toString() const
{
  char tmpBuf[64];
  sprintf(tmpBuf, "msgCode=%u", getMsgCode());

  std::string strBuf;
  if ( _isSetAffectedSSN ) {
    char ssnStr[32];
    sprintf(ssnStr, ",affectedSSN=%u", _affectedSSN);
    strBuf += ssnStr;
  }
  if ( _isSetAffectedPC ) {
    char pcStr[32];
    sprintf(pcStr, ",affectedPC=%u", _affectedPC);
    strBuf += pcStr;
  }
  if ( _isSetSmi ) {
    char smiStr[32];
    sprintf(smiStr, ",smi=%u", _smi);
    strBuf += smiStr;
  }

  return tmpBuf + strBuf;
}

size_t
SCMGMessage::addMFixedField(common::TP* result_buf, size_t offset, uint8_t value) const
{
  return common::addField(result_buf, offset, value);
}

size_t
SCMGMessage::addMFixedField(common::TP* result_buf, size_t offset, uint16_t value) const
{
  offset = common::addField(result_buf, offset, uint8_t(value & 0xFF));
  return common::addField(result_buf, offset, uint8_t((value >> 8) & 0xFF));
}

size_t
SCMGMessage::extractMFixedField(const common::TP& packet_buf, size_t offset,
                                uint8_t* value) const
{
  return common::extractField(packet_buf, offset, value);
}

size_t
SCMGMessage::extractMFixedField(const common::TP& packet_buf, size_t offset,
                                uint16_t* value) const
{
  uint8_t firstOctetValue;
  offset = common::extractField(packet_buf, offset, &firstOctetValue);
  uint8_t secondOctetValue;
  offset = common::extractField(packet_buf, offset, &secondOctetValue);
  *value = (secondOctetValue << 8) | firstOctetValue;
  return offset;
}

}}}}}}
