#include <stdio.h>

#include "eyeline/utilx/Exception.hpp"
#include "eyeline/ss7na/common/TP.hpp"
#include "N_PCSTATE_IND_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

N_PCSTATE_IND_Message::N_PCSTATE_IND_Message()
  : LibsccpMessage(_MSG_CODE),
    _fieldsMask(0), _affectedSignalingPoint(0),
    _signalingPointStatus(0), _remoteSCCPStatus(0)
{}

size_t
N_PCSTATE_IND_Message::serialize(common::TP* result_buf) const
{
  size_t offset = LibsccpMessage::serialize(result_buf);

  offset = common::addField(result_buf, offset, _fieldsMask);

  offset = common::addField(result_buf, offset, _affectedSignalingPoint);

  offset = common::addField(result_buf, offset, _signalingPointStatus);

  if ( _fieldsMask & SET_REMOTE_SCCP_STATUS )
    offset = common::addField(result_buf, offset, _remoteSCCPStatus);

  return offset;
}

size_t
N_PCSTATE_IND_Message::serialize(uint8_t* result_buf, size_t result_buf_max_sz) const
{
  common::TP tp(0, 0, result_buf, result_buf_max_sz);

  return serialize(&tp);
}

size_t
N_PCSTATE_IND_Message::deserialize(const common::TP& packet_buf)
{
  size_t offset = LibsccpMessage::deserialize(packet_buf);

  offset = common::extractField(packet_buf, offset, &_fieldsMask);

  offset = common::extractField(packet_buf, offset, &_affectedSignalingPoint);

  offset = common::extractField(packet_buf, offset, &_signalingPointStatus);

  if ( !_signalingPointStatus || _signalingPointStatus > SIGNALING_POINT_ACCESSIBLE )
    throw utilx::DeserializationException("N_PCSTATE_IND_Message::deserialize::: invalid value=%d of signalingPointStatus field - expected value must be in range [1-3]",
                                          _signalingPointStatus);

  if ( _fieldsMask & SET_REMOTE_SCCP_STATUS )
    offset = common::extractField(packet_buf, offset, &_remoteSCCPStatus);

  return offset;
}
size_t
N_PCSTATE_IND_Message::deserialize(const uint8_t* packet_buf, size_t packet_buf_sz)
{
  common::TP tp(0, packet_buf_sz, const_cast<uint8_t*>(packet_buf), packet_buf_sz);

  return deserialize(tp);
}

std::string
N_PCSTATE_IND_Message::toString() const
{
  std::string result(LibsccpMessage::toString());

  char strBuf[128];
  snprintf(strBuf, sizeof(strBuf), ",fieldsMask=[%02X]", _fieldsMask);
  result += strBuf;

  snprintf(strBuf, sizeof(strBuf), ",affectedSignalingPoint=[%d]", _affectedSignalingPoint);
  result += strBuf;

  snprintf(strBuf, sizeof(strBuf), ",signalingPointStatus=[%d]", _signalingPointStatus);
  result += strBuf;

  if ( _fieldsMask & SET_REMOTE_SCCP_STATUS ) {
    snprintf(strBuf, sizeof(strBuf), ",remoteSCCPStatus=[%d]", _remoteSCCPStatus);
    result += strBuf;
  }

  return result;
}

void
N_PCSTATE_IND_Message::setRemoteSCCPStatus(remote_sccp_status_e remote_SCCP_status)
{
  _remoteSCCPStatus = remote_SCCP_status;
  _fieldsMask |= SET_REMOTE_SCCP_STATUS;
}

N_PCSTATE_IND_Message::remote_sccp_status_e
N_PCSTATE_IND_Message::getRemoteSCCPStatus() const
{
  if ( _fieldsMask & SET_REMOTE_SCCP_STATUS )
    return remote_sccp_status_e(_remoteSCCPStatus);
  else
    throw utilx::FieldNotSetException("N_PCSTATE_IND_Message::getRemoteSCCPStatus::: remoteSCCPStatus wasn't set");
}

uint32_t
N_PCSTATE_IND_Message::getLength() const
{
  return LibsccpMessage::getLength() +
    static_cast<uint32_t>(sizeof(_fieldsMask) + sizeof(_affectedSignalingPoint) +
                          sizeof(_signalingPointStatus) + 
                          ( (_fieldsMask & SET_REMOTE_SCCP_STATUS) ? sizeof(_remoteSCCPStatus) : 0));
}

}}}
