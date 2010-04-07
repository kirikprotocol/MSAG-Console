#include <new>
#include "M3uaTLV.hpp"
#include "eyeline/utilx/hexdmp.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

std::string
TLV_NetworkAppearance::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "networkAppearance=[%u]", getValue());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_NetworkAppearance::toString::: value wasn't set");
}

TLV_UserCause::TLV_UserCause()
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_UserCause::TLV_UserCause(mtp_status_cause_e cause, mtp_user_identity_e user)
  : common::TLV_IntegerPrimitive(TAG, ((uint32_t)cause << 16) | user)
{}

size_t
TLV_UserCause::deserialize(const common::TP& packet_buf,
                           size_t offset,
                           uint16_t val_len)
{
  offset = common::TLV_IntegerPrimitive::deserialize(packet_buf, offset, val_len);
  uint16_t cause = getCause();
  if ( cause > InaccessibleRemoteUser )
    throw smsc::util::Exception("TLV_UserCause::deserialize::: wrong cause value=[%d], expected value from range [0-2]", cause);
  uint16_t user = getUser();
  if ( user > Reserved_15 )
    throw smsc::util::Exception("TLV_UserCause::deserialize::: wrong user value=[%d], expected value from range [0-15]", user);
  return offset;
}

std::string
TLV_UserCause::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "user_cause=[cause=[%d],user=[%d]]", getCause(), getUser());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_UserCause::toString::: value wasn't set");
}

mtp_user_identity_e
TLV_UserCause::getUser() const
{
  if ( !isSetValue() )
    throw utilx::FieldNotSetException("TLV_UserCause::getUser::: value isn't set");

  return mtp_user_identity_e((getValue() & 0x0000FFFF));
}

mtp_status_cause_e
TLV_UserCause::getCause() const
{
  if ( !isSetValue() )
    throw utilx::FieldNotSetException("TLV_UserCause::getCause::: value isn't set");

  return mtp_status_cause_e((getValue() >> 16) & 0x0000FFFF);

}

size_t
TLV_CongestionIndication::deserialize(const common::TP& packet_buf,
                                      size_t offset,
                                      uint16_t val_len)
{
  offset = common::TLV_IntegerPrimitive::deserialize(packet_buf, offset, val_len);
  uint32_t congestion = getValue();
  if ( congestion > CongestionLevel3 )
    throw smsc::util::Exception("TLV_CongestionIndication::deserialize::: wrong congestion level value=[%d], expected value from range [0x00-0x03]", congestion);
  return offset;
}

std::string
TLV_CongestionIndication::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "congestion=[%d]", getValue());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_CongestionIndication::toString::: value wasn't set");
}

std::string
TLV_ConcernedDestination::toString() const
{
  if ( isSetValue() ) {
    char strBuf[32];
    sprintf(strBuf, "point code=[0x%08X]", getValue());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_CongestionIndication::toString::: value wasn't set");
}

TLV_ProtocolData::TLV_ProtocolData()
: common::AdaptationLayer_TLV(TAG),
  _isSetOpc(false), _isSetDpc(false), _isSetSi(false), _isSetNi(false),
  _isSetMessagePriority(false), _isSetSls(false), _isSetUserProtocolData(false)
{}

size_t
TLV_ProtocolData::serialize(common::TP* packet_buf,
                            size_t offset) const
{
  if ( !isSetValue() )
    throw utilx::FieldNotSetException("TLV_ProtocolData::serialize::: value isn't set");

  offset = common::AdaptationLayer_TLV::serialize(packet_buf, offset);

  offset = common::addField(packet_buf, offset, _opc);
  offset = common::addField(packet_buf, offset, _dpc);
  offset = common::addField(packet_buf, offset, _si);
  offset = common::addField(packet_buf, offset, _ni);
  offset = common::addField(packet_buf, offset, _messagePriority);
  offset = common::addField(packet_buf, offset, _sls);
  return common::addField(packet_buf, offset, _userProtocolData.get(), _userProtocolData.getCurrentSize());
}

size_t
TLV_ProtocolData::deserialize(const common::TP& packet_buf,
                              size_t offset,
                              uint16_t val_len)
{
  offset = common::extractField(packet_buf, offset, &_opc);
  _isSetOpc = true;
  offset = common::extractField(packet_buf, offset, &_dpc);
  _isSetDpc = true;
  offset = common::extractField(packet_buf, offset, &_si);
  _isSetSi = true;
  offset = common::extractField(packet_buf, offset, &_ni);
  _isSetNi = true;
  offset = common::extractField(packet_buf, offset, &_messagePriority);
  _isSetMessagePriority = true;
  offset = common::extractField(packet_buf, offset, &_sls);
  _isSetSls = true;

  _userProtocolData.setSize(val_len - SIZE_OF_FIXED_FIELDS); _userProtocolData.rewind();
  _userProtocolData.append(&packet_buf.packetBody[offset], val_len - SIZE_OF_FIXED_FIELDS);
  _isSetUserProtocolData = true;

  return offset + val_len - SIZE_OF_FIXED_FIELDS;
}

uint16_t
TLV_ProtocolData::getLength() const
{
  return static_cast<uint16_t>(HEADER_SZ + SIZE_OF_FIXED_FIELDS + _userProtocolData.getCurrentSize());
}

bool
TLV_ProtocolData::isSetValue() const
{
  return _isSetOpc && _isSetDpc && _isSetSi && _isSetNi &&
         _isSetMessagePriority && _isSetSls && _isSetUserProtocolData;
}

std::string
TLV_ProtocolData::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "opc=[%u],dpc=[%u],si=[%u],ni=[%u],mp=[%u],sls=[%u],userProtocolData=[",
             _opc, _dpc, _si, _ni, _messagePriority, _sls);
    return strBuf + utilx::hexdmp(_userProtocolData.get(), _userProtocolData.getCurrentSize()) + ']';
  } else
    throw smsc::util::Exception("TLV_ProtocolData::toString::: value wasn't set");
}

uint32_t
TLV_ProtocolData::getOPC() const
{
  if ( !_isSetOpc )
    throw utilx::FieldNotSetException("TLV_ProtocolData::getOPC::: value isn't set");
  return _opc;
}

uint32_t
TLV_ProtocolData::getDPC() const
{
  if ( !_isSetDpc )
    throw utilx::FieldNotSetException("TLV_ProtocolData::getDPC::: value isn't set");
  return _dpc;
}

uint8_t
TLV_ProtocolData::getServiceIndicator() const
{
  if ( !_isSetSi )
    throw utilx::FieldNotSetException("TLV_ProtocolData::getServiceIndicator::: value isn't set");
  return _si;
}

uint8_t
TLV_ProtocolData::getNetworkIndicator() const
{
  if ( !_isSetSi )
    throw utilx::FieldNotSetException("TLV_ProtocolData::getNetworkIndicator::: value isn't set");
  return _ni;
}

uint8_t
TLV_ProtocolData::getMessagePriority() const
{
  if ( !_isSetMessagePriority )
    throw utilx::FieldNotSetException("TLV_ProtocolData::getMessagePriority::: value isn't set");
  return _messagePriority;
}

uint8_t
TLV_ProtocolData::getSLS() const
{
  if ( !_isSetSls )
    throw utilx::FieldNotSetException("TLV_ProtocolData::getSLS::: value isn't set");
  return _sls;
}

void
TLV_ProtocolData::setUserProtocolData(const uint8_t* val, uint16_t len) {
//  _isSetUserProtocolData = false;
  _userProtocolData.setValue(val, len);
//  _userProtocolData.setSize(len); _userProtocolData.rewind();
//  _userProtocolData.append(val, len);
  _isSetUserProtocolData = true;
}

utilx::variable_data_t
TLV_ProtocolData::getUserProtocolData() const
{
  if ( !_isSetUserProtocolData )
    throw utilx::FieldNotSetException("TLV_ProtocolData::getUserProtocolData::: value isn't set");
  return utilx::variable_data_t(_userProtocolData.get(), _userProtocolData.getCurrentSize());
}

unsigned int
M3uaTLVFactory::getTagIdx(uint16_t tag)
{
  if ( tag < 0x0014 )
    return tag;
  else if ( tag >= 0x0200 && tag < 0x0214 )
    return 0x0014 + tag - 0x0200;
  else throw smsc::util::Exception("M3uaTLVFactory::getTagIdx::: wrong tag value [=0x%04x]", tag);
}

}}}}}}
