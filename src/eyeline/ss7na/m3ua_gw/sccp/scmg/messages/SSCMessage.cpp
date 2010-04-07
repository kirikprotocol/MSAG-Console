#include "SSCMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {
namespace messages {

size_t
SSCMessage::serialize(common::TP* result_buf) const
{
  size_t offset = SCMGMessage::serialize(result_buf);

  if ( !_isSetCongestionLevel )
    throw utilx::SerializationException("SCMGMessage::serialize::: mandatory field congestionLevel isn't set");

  return  addMFixedField(result_buf, offset, uint8_t(_congestionLevel));
}

size_t
SSCMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = SCMGMessage::deserialize(packet_buf);

  uint8_t oneOctetValue;
  offset = extractMFixedField(packet_buf, offset, &oneOctetValue);
  if ( oneOctetValue > CongestionLevel_8 )
    throw utilx::DeserializationException("SSCMessage::deserialize::: invalid congestionLevel value=%d, expected value in range [0-8]",
                                          oneOctetValue);

  _congestionLevel = sccp_congestion_level_e(oneOctetValue);
  _isSetCongestionLevel = true;

  return offset;
}

std::string
SSCMessage::toString() const
{
  const std::string& strBuf = SCMGMessage::toString();

  if ( _isSetCongestionLevel ) {
    char congLevelStrBuf[32];
    sprintf(congLevelStrBuf, ",congestionLevel=%u", _congestionLevel);
    return strBuf + congLevelStrBuf;
  } else
    return strBuf;
}

}}}}}}
