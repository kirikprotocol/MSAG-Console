#include "N_PCSTATE_IND_Message.hpp"

namespace libsua_messages {

N_PCSTATE_IND_Message::N_PCSTATE_IND_Message()
  : LibsuaMessage(_MSG_CODE),
    _fieldsMask(0), _affectedSignalingPoint(0),
    _signalingPointStatus(0), _remoteSCCPStatus(0)
{}

size_t
N_PCSTATE_IND_Message::serialize(communication::TP* resultBuf) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf);

  offset = communication::addField(resultBuf, offset, _fieldsMask);

  offset = communication::addField(resultBuf, offset, _affectedSignalingPoint);

  offset = communication::addField(resultBuf, offset, _signalingPointStatus);

  if ( _fieldsMask & SET_REMOTE_SCCP_STATUS )
    offset = communication::addField(resultBuf, offset, _remoteSCCPStatus);

  return offset;
}

size_t
N_PCSTATE_IND_Message::deserialize(const communication::TP& packetBuf)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf);

  offset = communication::extractField(packetBuf, offset, &_fieldsMask);

  offset = communication::extractField(packetBuf, offset, &_affectedSignalingPoint);

  offset = communication::extractField(packetBuf, offset, &_signalingPointStatus);

  if ( _fieldsMask & SET_REMOTE_SCCP_STATUS )
    offset = communication::extractField(packetBuf, offset, &_remoteSCCPStatus);

  return offset;
}

std::string
N_PCSTATE_IND_Message::toString() const
{
  std::string result(LibsuaMessage::toString());

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

const char*
N_PCSTATE_IND_Message::getMsgCodeTextDescription() const
{
  return "N_PCSTATE_IND_Message";
}

void
N_PCSTATE_IND_Message::setAffectedSignalingPoint(uint16_t pointCode)
{
  _affectedSignalingPoint = pointCode;
}

uint16_t
N_PCSTATE_IND_Message::getAffectedSignalingPoint() const
{
  return _affectedSignalingPoint;
}

void
N_PCSTATE_IND_Message::setSignalingPointStatus(uint8_t signalingPointStatus)
{
  _signalingPointStatus = signalingPointStatus;
}

uint8_t
N_PCSTATE_IND_Message::getSignalingPointStatus() const
{
  return _signalingPointStatus;
}

void
N_PCSTATE_IND_Message::setRemoteSCCPStatus(uint8_t remoteSCCPStatus)
{
  _remoteSCCPStatus = remoteSCCPStatus;
  _fieldsMask |= SET_REMOTE_SCCP_STATUS;
}

uint8_t
N_PCSTATE_IND_Message::getRemoteSCCPStatus() const
{
  if ( _fieldsMask & SET_REMOTE_SCCP_STATUS )
    return _remoteSCCPStatus;
  else
    throw utilx::FieldNotSetException("N_PCSTATE_IND_Message::getRemoteSCCPStatus::: remoteSCCPStatus wasn't set");
}

uint32_t
N_PCSTATE_IND_Message::getLength() const
{
  return
    LibsuaMessage::getLength() + sizeof(_fieldsMask) + sizeof(_affectedSignalingPoint) +
    sizeof(_signalingPointStatus) + 
    ( (_fieldsMask & SET_REMOTE_SCCP_STATUS) ? sizeof(_remoteSCCPStatus) : 0 );
}

}
